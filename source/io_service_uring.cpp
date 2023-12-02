#include "toyws/io_service_uring.hpp"

#include <arpa/inet.h>
#include <fmt/core.h>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cassert>
#include <cerrno>
#include <cstring>

#include "toyws/error.hpp"
#include "toyws/request.hpp"
#include "toyws/toyws.hpp"

toyws::IoService::IoService()
    : ring{std::make_unique<io_uring>()},
      address{"127.0.0.1"},
      port{5000},
      clientNameLen{sizeof(sockaddr_in)},
      submitAlways{true} {
  requests.resize(kSqSize + kCqSize);
  bufferDescriptors.resize(kSqSize + kCqSize);
}

toyws::IoService::~IoService() {
  if (ring) {
    io_uring_queue_exit(ring.get());
  }
}

auto toyws::IoService::Run() -> void {
  MakeListeningSocket();

  CreateIoRing();

  AsyncAccept();

  while (true) {
    io_uring_cqe* cqe;
    if (int res = io_uring_wait_cqe(ring.get(), &cqe); res != 0) {
      throw Error(
          fmt::format("Error in io_uring_wait_cqe(): {}", std::strerror(-res)));
    }

    submitAlways = false;

    while (true) {
      HandleCqe(cqe);

      io_uring_cqe_seen(ring.get(), cqe);
      if (io_uring_peek_cqe(ring.get(), &cqe) == -EAGAIN) {
        break;
      }
    }

    if (submissions > 0) {
      ForceSubmit();
    }

    submitAlways = true;
  }
}

auto toyws::IoService::Stop() -> void {
  io_uring_queue_exit(ring.get());
  ring = nullptr;
}

auto toyws::IoService::MakeListeningSocket() -> void {
  // Create socket file descriptor
  listeningFd = socket(PF_INET, SOCK_STREAM, 0);
  if (listeningFd == -1) {
    throw Error(fmt::format("Error in socket(): {}", std::strerror(errno)));
  }

  // Allow address reuse (for quick server restarts)
  const int on = 1;
  if (setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) ==
      -1) {
    throw Error(fmt::format("Error in setsockopt(): {}", std::strerror(errno)));
  }

  // Assign name to socket
  sockaddr_in name{};
  name.sin_family = AF_INET;
  name.sin_port = htons(port);
  if (inet_pton(AF_INET, address.c_str(), &name.sin_addr) != 1) {
    throw Error("Invalid network address");
  }
  if (bind(listeningFd, reinterpret_cast<const sockaddr*>(&name),
           sizeof(name)) == -1) {
    throw Error(fmt::format("Error in bind(): {}", std::strerror(errno)));
  }

  // Listen
  if (listen(listeningFd, kAcceptQueue) == -1) {
    throw Error(fmt::format("Error in listen(): {}", std::strerror(errno)));
  }
}

auto toyws::IoService::CreateIoRing() -> void {
  // TODO: Look into IORING_SETUP_SQPOLL
  io_uring_params params{};
  params.cq_entries = kCqSize;

  if (auto res = io_uring_queue_init_params(kSqSize, ring.get(), &params);
      res < 0) {
    throw Error(fmt::format("Error in io_uring_queue_init_params(): {}",
                            std::strerror(-res)));
  }
}

auto toyws::IoService::AsyncAccept() -> void {
  auto* sqe = io_uring_get_sqe(ring.get());
  assert(sqe != nullptr);  // null if SQ is full

  io_uring_prep_accept(sqe, listeningFd,
                       reinterpret_cast<sockaddr*>(clientName.get()),
                       &clientNameLen, 0);

  auto request = ToyWs::Instance()->RequestPool().Acquire();
  const std::size_t slot = nextReqSlot;
  nextReqSlot = (nextReqSlot + 1) % requests.size();
  requests[slot] = std::move(request);
  io_uring_sqe_set_data64(sqe, slot);

  Submit();
}

auto toyws::IoService::AsyncRead(const std::size_t reqSlot) -> void {
  auto* sqe = io_uring_get_sqe(ring.get());
  assert(sqe != nullptr);  // null if SQ is full

  auto& request = requests[reqSlot];
  bufferDescriptors[reqSlot].iov_base = request->Buffer().data();
  bufferDescriptors[reqSlot].iov_len = request->Buffer().size();
  io_uring_prep_readv(sqe, request->Socket(), &bufferDescriptors[reqSlot], 1,
                      0);
  io_uring_sqe_set_data64(sqe, reqSlot);
  request->SetState(Request::States::kRead);

  Submit();
}

auto toyws::IoService::AsyncWrite(const std::size_t reqSlot) -> void {
  auto* sqe = io_uring_get_sqe(ring.get());
  assert(sqe != nullptr);  // null if SQ is full

  auto& request = requests[reqSlot];
  bufferDescriptors[reqSlot].iov_base = request->Buffer().data();
  bufferDescriptors[reqSlot].iov_len = request->BufferContentSize();
  io_uring_prep_writev(sqe, request->Socket(), &bufferDescriptors[reqSlot], 1,
                       0);
  io_uring_sqe_set_data64(sqe, reqSlot);
  request->SetState(Request::States::kWrite);

  Submit();
}

auto toyws::IoService::ForceSubmit() -> void {
  io_uring_submit(ring.get());
  submissions = 0;
}

auto toyws::IoService::HandleCqe(io_uring_cqe* cqe) -> void {
  // TODO: Should not automatically throw on error
  if (cqe->res < 0) {
    throw Error(
        fmt::format("Error in async step: {}", std::strerror(-cqe->res)));
  }

  const auto slot = static_cast<std::size_t>(cqe->user_data);
  auto& request = requests[slot];
  switch (request->State()) {
    case Request::States::kAccept:
      request->SetSocket(cqe->res);
      AsyncAccept();
      AsyncRead(slot);
      break;
    case Request::States::kRead:
      // TODO: Keep reading if there's still stuff to be read
      if (cqe->res == 0) {
        // TODO: Handle empty request
      }
      assert(cqe->res >= 0);
      request->SetBufferContentSize(static_cast<std::size_t>(cqe->res));
      AsyncWrite(slot);
      break;
    case Request::States::kWrite:
      // TODO: only remove if all writing is finished
      close(request->Socket());
      requests[slot] = nullptr;
      break;
    default:
      assert(false && "Unhandled Request::State in HandleCqe");
      break;
  }
}

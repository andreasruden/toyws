#include <arpa/inet.h>
#include <sys/socket.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <format>

#include "toyws/client.hpp"
#include "toyws/error.hpp"
#include "toyws/io_service.hpp"
#include "toyws/toyws.hpp"

template <typename Handler>
toyws::IoService<Handler>::IoService() {
  clients.resize(kSqSize + kCqSize);
  bufferDescriptors.resize(kSqSize + kCqSize);

  CreateIoRing();
}

template <typename Handler>
toyws::IoService<Handler>::~IoService() {
  io_uring_queue_exit(&ring);
}

template <typename Handler>
auto toyws::IoService<Handler>::Run() -> void {
  // TODO: Add dummy fd as an event based wakeup, to notice running = false
  running = true;
  while (running) {
    io_uring_cqe* cqe;
    if (int res = io_uring_wait_cqe(&ring, &cqe); res != 0) {
      throw Error(
          std::format("Error in io_uring_wait_cqe(): {}", std::strerror(-res)));
    }

    submitAlways = false;

    while (true) {
      HandleCqe(cqe);

      io_uring_cqe_seen(&ring, cqe);
      if (io_uring_peek_cqe(&ring, &cqe) == -EAGAIN) {
        break;
      }
    }

    if (submissions > 0) {
      ForceSubmit();
    }

    submitAlways = true;
  }
}

template <typename Handler>
auto toyws::IoService<Handler>::Stop() -> void {
  running = false;
}

template <typename Handler>
auto toyws::IoService<Handler>::MakeListeningSocket(std::string address,
                                                    uint16_t port) -> Socket {
  // Create socket file descriptor
  int listeningFd = socket(PF_INET, SOCK_STREAM, 0);
  if (listeningFd == -1) {
    throw Error(std::format("Error in socket(): {}", std::strerror(errno)));
  }

  // Allow address reuse (for quick server restarts)
  const int on = 1;
  if (setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) ==
      -1) {
    throw Error(std::format("Error in setsockopt(): {}", std::strerror(errno)));
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
    throw Error(std::format("Error in bind(): {}", std::strerror(errno)));
  }

  // Listen
  if (listen(listeningFd, kAcceptQueue) == -1) {
    throw Error(std::format("Error in listen(): {}", std::strerror(errno)));
  }

  return listeningFd;
}

template <typename Handler>
auto toyws::IoService<Handler>::CreateIoRing() -> void {
  // TODO: Look into IORING_SETUP_SQPOLL
  io_uring_params params{};
  params.cq_entries = kCqSize;

  if (auto res = io_uring_queue_init_params(kSqSize, &ring, &params); res < 0) {
    throw Error(std::format("Error in io_uring_queue_init_params(): {}",
                            std::strerror(-res)));
  }
}

template <typename Handler>
auto toyws::IoService<Handler>::AsyncAccept(Socket listeningFd) -> void {
  auto* sqe = io_uring_get_sqe(&ring);
  assert(sqe != nullptr);  // null if SQ is full

  io_uring_prep_accept(sqe, listeningFd,
                       reinterpret_cast<sockaddr*>(&clientName), &clientNameLen,
                       0);

  auto client = clientPool.Acquire();
  // FIXME: Ensure we get an empty slot
  const std::size_t slot = nextClientSlot;
  nextClientSlot = (nextClientSlot + 1) % clients.size();
  client->SetSocket(listeningFd);
  client->SetIoServiceSlot(static_cast<int>(slot));
  clients[slot] = std::move(client);
  io_uring_sqe_set_data64(sqe, slot);

  Submit();
}

template <typename Handler>
auto toyws::IoService<Handler>::AsyncRead(int clientSlot) -> void {
  assert(clientSlot >= 0);

  auto* sqe = io_uring_get_sqe(&ring);
  assert(sqe != nullptr);  // null if SQ is full

  auto slot = static_cast<std::size_t>(clientSlot);
  auto& client = clients[slot];
  bufferDescriptors[slot].iov_base = client->Buffer().data();
  bufferDescriptors[slot].iov_len = client->Buffer().size();
  io_uring_prep_readv(sqe, client->Socket(), &bufferDescriptors[slot], 1, 0);
  io_uring_sqe_set_data64(sqe, slot);
  client->SetState(Client::States::kRead);

  Submit();
}

template <typename Handler>
auto toyws::IoService<Handler>::AsyncWrite(int clientSlot) -> void {
  assert(clientSlot >= 0);

  auto* sqe = io_uring_get_sqe(&ring);
  assert(sqe != nullptr);  // null if SQ is full

  auto slot = static_cast<std::size_t>(clientSlot);
  auto& client = clients[slot];
  bufferDescriptors[slot].iov_base = client->Buffer().data();
  bufferDescriptors[slot].iov_len = client->BufferContentSize();
  io_uring_prep_writev(sqe, client->Socket(), &bufferDescriptors[slot], 1, 0);
  io_uring_sqe_set_data64(sqe, slot);
  client->SetState(Client::States::kWrite);

  Submit();
}

template <typename Handler>
auto toyws::IoService<Handler>::TakeClient(int clientSlot)
    -> std::unique_ptr<Client> {
  assert(clientSlot >= 0);

  auto slot = static_cast<std::size_t>(clientSlot);
  auto ptr = std::move(clients[slot]);
  clients[slot] = nullptr;
  ptr->SetIoServiceSlot(-1);

  return ptr;
}

template <typename Handler>
auto toyws::IoService<Handler>::GiveClient(std::unique_ptr<Client> client)
    -> void {
  // FIXME: Ensure we get an empty slot
  const std::size_t slot = nextClientSlot;
  nextClientSlot = (nextClientSlot + 1) % clients.size();
  client->SetIoServiceSlot(static_cast<int>(slot));
  clients[slot] = std::move(client);
}

template <typename Handler>
auto toyws::IoService<Handler>::Close(Client* client) -> void {
  auto slot = static_cast<std::size_t>(client->IoServiceSlot());
  assert(clients[slot].get() == client);

  close(client->Socket());
  clients[slot] = nullptr;
}

template <typename Handler>
auto toyws::IoService<Handler>::ForceSubmit() -> void {
  io_uring_submit(&ring);
  submissions = 0;
}

template <typename Handler>
auto toyws::IoService<Handler>::HandleCqe(io_uring_cqe* cqe) -> void {
  // TODO: Should not automatically throw on error
  if (cqe->res < 0) {
    throw Error(
        std::format("Error in async step: {}", std::strerror(-cqe->res)));
  }

  const auto slot = static_cast<std::size_t>(cqe->user_data);
  auto& client = clients[slot];
  assert(static_cast<int>(slot) == client->IoServiceSlot());
  switch (client->State()) {
    case Client::States::kAccept: {
      Socket listeningFd = client->Socket();
      client->SetSocket(cqe->res);
      Handler::OnAccept(this, listeningFd, client.get());
      break;
    }
    case Client::States::kRead:
      // TODO: Keep reading if there's still stuff to be read
      if (cqe->res == 0) {
        // TODO: Handle error
      }
      assert(cqe->res >= 0);
      client->SetBufferContentSize(static_cast<std::size_t>(cqe->res));
      Handler::OnRead(this, client.get());
      break;
    case Client::States::kWrite:
      // TODO: Verify all was wr itten?
      Handler::OnWrite(this, client.get());
      break;
    default:
      assert(false && "Unhandled Request::State in HandleCqe");
      break;
  }
}

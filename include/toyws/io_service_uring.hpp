#pragma once

#include <sys/uio.h>

#include <memory>
#include <vector>

struct io_uring;
struct io_uring_cqe;
struct sockaddr_in;

namespace toyws {

class Request;

inline constexpr int kAcceptQueue = 5;
inline constexpr int kSqSize = 16;
inline constexpr int kCqSize = 64;

class IoService {
 public:
  explicit IoService();

  ~IoService();

  // TODO: prevent copying, etc

  auto Run() -> void;

  auto Stop() -> void;

 private:
  std::unique_ptr<io_uring> ring;
  std::string address;
  unsigned short port;
  int listeningFd;
  std::unique_ptr<sockaddr_in> clientName;
  unsigned int clientNameLen;
  int submissions;
  bool submitAlways;

  std::size_t nextReqSlot;
  std::vector<std::unique_ptr<Request>> requests;
  std::vector<iovec> bufferDescriptors;

  auto MakeListeningSocket() -> void;

  auto CreateIoRing() -> void;

  auto AsyncAccept() -> void;

  auto AsyncRead(std::size_t reqSlot) -> void;

  auto AsyncWrite(std::size_t reqSlot) -> void;

  auto ShouldSubmit() const -> bool {
    return submitAlways || submissions >= kSqSize;
  }

  auto Submit() -> void {
    ++submissions;
    if (ShouldSubmit()) {
      ForceSubmit();
    }
  }

  auto ForceSubmit() -> void;

  auto HandleCqe(io_uring_cqe* cqe) -> void;
};

}  // namespace toyws

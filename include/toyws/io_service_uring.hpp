#pragma once

#include <liburing.h>
#include <netinet/in.h>
#include <sys/uio.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "toyws/client_pool.hpp"
#include "toyws/socket.hpp"

namespace toyws {

class Client;
class ToyWs;

inline constexpr int kAcceptQueue = 5;
inline constexpr int kSqSize = 16;
inline constexpr int kCqSize = 64;

template <typename Handler>
class IoService {
 public:
  explicit IoService();

  ~IoService();

  // TODO: prevent copying, etc

  auto Run() -> void;

  auto Stop() -> void;

  auto MakeListeningSocket(std::string address, uint16_t port) -> Socket;

  auto AsyncAccept(Socket listeningFd) -> void;

  // Precondition: Client must exist in IoService already.
  //               Either via AsyncAccept() or GiveClient().
  auto AsyncRead(int clientSlot) -> void;

  // Precondition: Client must exist in IoService already.
  //               Either via AsyncAccept() or GiveClient().
  auto AsyncWrite(int clientSlot) -> void;

  auto TakeClient(int clientSlot) -> std::unique_ptr<Client>;

  auto GiveClient(std::unique_ptr<Client> client) -> void;

  // Shorthand for: TakeClient() and then client.Socket().close()
  auto Close(Client* client) -> void;

  auto SetInstance(ToyWs* parent) -> void { parentInst = parent; }
  auto Instance() const -> ToyWs* { return parentInst; }

 private:
  io_uring ring = {};
  sockaddr_in clientName = {};
  unsigned int clientNameLen = sizeof(sockaddr_in);
  int submissions = 0;
  bool submitAlways = true;
  bool running = false;

  ClientPool clientPool;
  std::size_t nextClientSlot = 0;
  std::vector<std::unique_ptr<Client>> clients;
  std::vector<iovec> bufferDescriptors;

  ToyWs* parentInst;

  auto CreateIoRing() -> void;

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

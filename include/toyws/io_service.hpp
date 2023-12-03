#pragma once

#include "io_service_uring.hpp"

/*
\**
 * @brief Asynchronous Input / Output handler
 *
 * Provides Input / Output handling. Templated on "Handler" by
 * "IoServiceHandler" (see below). Directly include "io_service.cpp" into T
 * implementation instead of this file.
 *\
template <typename Handler>
class IoService {
 public:
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

  auto SetInstance(ToyWs* parent) -> void;
  auto Instance() const -> ToyWs*;
};
*/

/*class IoServiceHandler {
 public:
  static auto OnAccept(IoService* service, Socket listenSock, Client* client) ->
    void;

  static auto OnRead(IoService* service, Client* client) -> void;

  static auto OnWrite(IoService* service, Client* client) -> void;
};*/

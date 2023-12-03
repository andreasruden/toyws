#pragma once

#include "toyws/io_service.hpp"

namespace toyws {

/**
 * @brief Handler for use with IoService.
 *
 * Handles receiving a request from a client, dispatching it to the ToyWs
 * instance and returning the response.
 *
 * TODO: It is not meant to also handle listening. But currently it does.
 */
class RequestHandler {
 public:
  static auto OnAccept(IoService<RequestHandler>* service, Socket listenSock,
                       Client* client) -> void;

  static auto OnRead(IoService<RequestHandler>* service, Client* client)
      -> void;

  static auto OnWrite(IoService<RequestHandler>* service, Client* client)
      -> void;
};

}  // namespace toyws

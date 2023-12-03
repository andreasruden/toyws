#pragma once

#include <cstdint>
#include <string>

#include "toyws/http_request.hpp"
#include "toyws/http_response.hpp"
#include "toyws/io_service.hpp"
#include "toyws/request_handler.hpp"
#include "toyws/toyws_export.hpp"

namespace toyws {

/**
 * @brief Main app class for using Toy Web Server.
 */
class TOYWS_EXPORT ToyWs {
 public:
  ToyWs(std::string address, uint16_t port);

  auto Run() -> void;

  auto Stop() -> void;

  auto HandleRequest(const HttpRequest& request) -> HttpResponse;

 private:
  std::string listeningAddress;
  uint16_t listeningPort;
  IoService<RequestHandler> ioService;

  // Suppress MSVC warning about containing data types that are not exported.
  // This is okay if non-exported types are private members & not part of API.
  // TOYWS_SUPPRESS_C4251
};

}  // namespace toyws

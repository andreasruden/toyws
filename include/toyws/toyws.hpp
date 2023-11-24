#pragma once

#include <string>

#include "toyws/io_service.hpp"
#include "toyws/request_pool.hpp"
#include "toyws/toyws_export.hpp"

namespace toyws {

/**
 * @brief Main app class for using Toy Web Server. There may exist only one of
 * these per process.
 */
class TOYWS_EXPORT ToyWs {
 public:
  ToyWs();

  ~ToyWs();

  static auto Instance() -> ToyWs* { return instance; }

  auto RequestPool() -> RequestPool& { return requestPool; }

  auto Run() -> void { ioService.Run(); }

  auto Stop() -> void { ioService.Stop(); }

 private:
  static ToyWs* instance;  // singleton reference; no ownership
                           // TODO: thread-safety

  // Suppress MSVC warning about containing data types that are not exported.
  // This is okay if non-exported types are private members & not part of API.
  TOYWS_SUPPRESS_C4251
  IoService ioService;
  class RequestPool requestPool;
};

}  // namespace toyws

#pragma once

#include <memory>

#include "toyws/toyws_export.hpp"

namespace toyws {

class Client;

class TOYWS_EXPORT ClientPool {
 public:
  auto Acquire() -> std::unique_ptr<Client>;
};

}  // namespace toyws

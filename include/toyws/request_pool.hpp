#pragma once

#include <memory>

#include "toyws/toyws_export.hpp"

namespace toyws {

class Request;

class TOYWS_EXPORT RequestPool {
 public:
  auto Acquire() -> std::unique_ptr<Request>;
};

}  // namespace toyws

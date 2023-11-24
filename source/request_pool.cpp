#include "toyws/request_pool.hpp"

#include "toyws/request.hpp"

auto toyws::RequestPool::Acquire() -> std::unique_ptr<Request> {
  // TODO: pool implementation :-)
  return std::make_unique<Request>();
}

#include "toyws/client_pool.hpp"

#include "toyws/client.hpp"

auto toyws::ClientPool::Acquire() -> std::unique_ptr<Client> {
  // TODO: pool implementation :-)
  return std::make_unique<Client>();
}

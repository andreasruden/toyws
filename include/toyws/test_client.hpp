#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "toyws/http_headers_map.hpp"
#include "toyws/http_request.hpp"
#include "toyws/http_response.hpp"

namespace toyws {

class TestClient {
 public:
  explicit TestClient(std::string serverAddress = "127.0.0.1",
                      const std::uint16_t serverPort = 5000)
      : address{std::move(serverAddress)}, port{serverPort} {}

  auto Headers() -> HeadersMap& { return headers; }
  auto Headers() const -> const HeadersMap& { return headers; }

  auto Request(HttpMethod method, const std::string& route) -> HttpResponse;

  auto Get(const std::string& route) -> HttpResponse {
    return Request(HttpMethod::GET, route);
  }

 private:
  std::string address;
  std::uint16_t port;
  HeadersMap headers;
  int socketFd;

  auto Connect() -> void;

  auto Send(const char* data, std::size_t length) -> void;

  auto Recv(char* data, std::size_t capacity) -> std::size_t;
};

}  // namespace toyws

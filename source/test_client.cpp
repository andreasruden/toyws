#include "toyws/test_client.hpp"

#include <arpa/inet.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "toyws/error.hpp"

auto toyws::TestClient::Request(HttpMethod method, const std::string& route)
    -> HttpResponse {
  constexpr std::size_t bufSize = 2048;
  char buf[bufSize];

  // Send Request
  HttpRequest request{method, route};
  std::pair<bool, std::size_t> res;
  do {
    res = request.Write(buf, bufSize);
    Send(buf, res.second);
  } while (!res.first);

  // Receive response
  HttpResponse response;
  bool recvRes;
  do {
    auto sz = Recv(buf, bufSize);
    recvRes = response.Read(buf, sz);
  } while (!recvRes);

  return response;
}

auto toyws::TestClient::Connect() -> void {
  sockaddr_in name{};
  name.sin_family = AF_INET;
  name.sin_port = htons(port);
  if (inet_pton(AF_INET, address.c_str(), &name.sin_addr) != 1) {
    throw Error("Invalid network address");
  }

  socketFd = socket(PF_INET, SOCK_STREAM, 0);
  if (socketFd == -1) {
    throw Error(
        fmt::format("TestClient error in socket(): {}", std::strerror(errno)));
  }
  if (connect(socketFd, reinterpret_cast<const sockaddr*>(&name),
              sizeof(name)) == -1) {
    throw Error(
        fmt::format("TestClient error in connect(): {}", std::strerror(errno)));
  }
}

auto toyws::TestClient::Send(const char* data, std::size_t length) -> void {
  if (socketFd == 0) {
    Connect();
  }

  if (send(socketFd, static_cast<const void*>(data), length, 0) == -1) {
    throw Error(
        fmt::format("TestClient error in send(): {}", std::strerror(errno)));
  }
}

auto toyws::TestClient::Recv(char* data, std::size_t capacity) -> std::size_t {
  if (socketFd == 0) {
    Connect();
  }

  ssize_t sz = recv(socketFd, static_cast<void*>(data), capacity, 0);
  if (sz == -1) {
    throw Error(
        fmt::format("TestClient error in send(): {}", std::strerror(errno)));
  }

  return static_cast<std::size_t>(sz);
}

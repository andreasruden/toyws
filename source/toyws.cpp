#include "toyws/toyws.hpp"

#include <format>
#include <iostream>

#include "toyws/http_request.hpp"
#include "toyws/http_response.hpp"

toyws::ToyWs::ToyWs(std::string address, uint16_t port)
    : listeningAddress{std::move(address)}, listeningPort{port} {}

auto toyws::ToyWs::Run() -> void {
  auto socket = ioService.MakeListeningSocket(listeningAddress, listeningPort);
  ioService.AsyncAccept(socket);
  ioService.Run();
}

auto toyws::ToyWs::Stop() -> void { ioService.Stop(); }

auto toyws::ToyWs::HandleRequest(const HttpRequest& request)
    -> toyws::HttpResponse {
  // TODO: Logging instead of cout
  std::cout << std::format("[{}] {} {}\n", "TODO: client addr",
                           HttpMethodName(request.Method()),
                           request.Resource());
  HttpResponse response{HttpStatus::kOk};
  return response;
}

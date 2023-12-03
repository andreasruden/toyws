#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

#include "toyws/error.hpp"
#include "toyws/io_service_impl.hpp"
#include "toyws/test_client.hpp"

// TODO: Figure out how to solve testing the different IoService
// implementations. Which is made more difficult by them being platform
// dependent.

/**
 * @brief We randomize ports to facilitate running test in parallel.
 */
auto RandomPort() -> uint16_t {
  static std::random_device randDevice;
  static std::mt19937 mt(randDevice());
  static std::uniform_int_distribution<uint16_t> distribution(1024, 65535);
  return distribution(mt);
}

/**
 * @brief Echo server implementation using IoService
 */
class EchoHandler {
 public:
  static auto OnAccept(toyws::IoService<EchoHandler>* service,
                       toyws::Socket listeningFd, toyws::Client* client)
      -> void {
    service->AsyncAccept(listeningFd);
    service->AsyncRead(client->IoServiceSlot());
  }

  static auto OnRead(toyws::IoService<EchoHandler>* service,
                     toyws::Client* client) -> void {
    service->AsyncWrite(client->IoServiceSlot());
  }

  static auto OnWrite(toyws::IoService<EchoHandler>* service,
                      toyws::Client* client) -> void {
    service->Close(client);
    service->Stop();
  }
};
namespace toyws {
template class IoService<EchoHandler>;
}

/**
 * @brief Basic HTTP response handler using IoService
 */
class HttpBasicHandler {
 public:
  static auto OnAccept(toyws::IoService<HttpBasicHandler>* service,
                       toyws::Socket listeningFd, toyws::Client* client)
      -> void {
    service->AsyncAccept(listeningFd);
    service->AsyncRead(client->IoServiceSlot());
  }

  static auto OnRead(toyws::IoService<HttpBasicHandler>* service,
                     toyws::Client* client) -> void {
    toyws::HttpResponse response{toyws::HttpStatus::kOk, "All Good"};
    auto res = response.Write(client->Buffer().data(), client->Buffer().size());
    client->SetBufferContentSize(res.second);
    service->AsyncWrite(client->IoServiceSlot());
  }

  static auto OnWrite(toyws::IoService<HttpBasicHandler>* service,
                      toyws::Client* client) -> void {
    service->Close(client);
    service->Stop();
  }
};
namespace toyws {
template class IoService<HttpBasicHandler>;
}

/**
 * @brief Io service fixture. Runs IoService in seperate thread for easier
 * tests.
 */
template <typename Handler>
struct IoServiceFixture {
  toyws::IoService<Handler> service;
  std::thread thread;
  uint16_t port;

  IoServiceFixture() {
    int maxTries = 5;
    toyws::Socket sock = -1;
    while (maxTries-- > 0) {
      try {
        port = RandomPort();
        sock = service.MakeListeningSocket("127.0.0.1", port);
        break;
      } catch (const toyws::Error& e) {
        // Do nothing
      }
    }
    if (sock == -1) {
      throw std::runtime_error("Test failed binding to random port 5 times");
    }
    service.AsyncAccept(sock);
    thread = std::thread{[&] {
      try {
        service.Run();
      } catch (const toyws::Error& err) {
        // Do Nothing
      }
    }};
  }

  ~IoServiceFixture() {
    if (thread.joinable()) {
      thread.join();
    }
  }
};

TEST_CASE("IoService simple echo", "[library]") {
  IoServiceFixture<EchoHandler> service;

  toyws::TestClient client{service.port};
  auto response = client.RawRequest("Hello There", 32);

  REQUIRE(response == "Hello There");
}

TEST_CASE("IoService + TestClient HTTP exchange", "[library]") {
  IoServiceFixture<HttpBasicHandler> service;

  toyws::TestClient client{service.port};
  auto response = client.Get("/");

  REQUIRE(response.Status() == toyws::HttpStatus::kOk);
  REQUIRE(response.Reason() == "All Good");
}

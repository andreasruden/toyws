#include <fmt/core.h>

#include <csignal>

#include "toyws/error.hpp"
#include "toyws/toyws.hpp"

toyws::ToyWs instance{"127.0.0.1", 5000};

auto SigIntHandler(int signal) -> void {
  if (signal == SIGINT) {
    instance.Stop();
  }
}

auto main() -> int {
  std::signal(SIGINT, SigIntHandler);

  fmt::print("Echo Server! Listening at port 5000...\n");
  try {
    instance.Run();
  } catch (const toyws::Error& e) {
    fmt::print("Error in Run(): {}\n", e.what());
  }

  fmt::print("Shutdown!\n");
}

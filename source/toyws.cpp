#include "toyws/toyws.hpp"

#include <cassert>
#include <string>

#include "toyws/error.hpp"

toyws::ToyWs* toyws::ToyWs::instance = nullptr;

toyws::ToyWs::ToyWs() {
  if (instance != nullptr) {
    throw Error("There may only be one ToyWs instance");
  }

  instance = this;
}

toyws::ToyWs::~ToyWs() {
  assert(instance == this);
  instance = nullptr;
}

#pragma once

#include <stdexcept>

#include "toyws/toyws_export.hpp"

namespace toyws {

class TOYWS_EXPORT Error : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace toyws

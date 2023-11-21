#include "toyws/toyws.hpp"

#include <fmt/core.h>

#include <string>

ExportedClass::ExportedClass() : name{fmt::format("{}", "toyws")} {}

auto ExportedClass::Name() const -> char const* { return name.c_str(); }

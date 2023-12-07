#include "toyws/router.hpp"

#include "toyws/error.hpp"

toyws::Route::Route(std::string path) : fullPath{std::move(path)} {
  auto slash = fullPath.rfind('/');
  if (slash == std::string::npos) {
    throw Error(std::format("Path {} must contain at least one '/'", fullPath));
  }
  identifier = fullPath.substr(slash);
}

auto toyws::Route::Match(const std::string& uri) -> Route* {
  if (!uri.starts_with(identifier)) {
    return nullptr;
  }

  auto rest = uri.substr(identifier.size());
  if (rest.empty()) {
    return this;
  }

  for (auto& route : subRoutes) {
    if (route.AppliesTo(rest)) {
      auto match = route.Matches(rest);
      if (match != nullptr) {
        return match;
      }
    }
  }

  return nullptr;
}

auto toyws::Route::PartialMatch(const std::string& uri) -> Route* {
  if (!uri.starts_with(identifier)) {
    return this;
  }

  auto rest = uri.substr(identifier.size());
  if (rest.empty()) {
    return this;
  }

  for (auto& route : subRoutes) {
    if (route.AppliesTo(rest)) {
      return route.BestMatch(rest);
    }
  }

  return this;
}

auto toyws::Router::AddRoute(const std::string& uri, SyncHandler handler)
    -> void {
  ids = SplitUri(uri);
  if (auto route = FindPartialMatchingRoute(uri); route) {
  } else {
    Route newRoute{};
  }
}

auto toyws::Router::FindRoute(const std::string& uri) -> Route* {
  for (auto& route : routes) {
    if (!route.AppliesTo(uri)) {
      continue;
    }
    auto ptr = route.Matches(uri);
    if (ptr != null) {
      return ptr;
    }
  }
  return nullptr;
}

auto toyws::Router::FindPartialMatchingRoute(const std::string& uri) -> Route* {
  for (auto& route : routes) {
    if (route.AppliesTo(uri)) {
      return route.BestMatch(uri);
    }
  }
  return nullptr;
}

auto toyws::Router::SplitUri(const std::string& uri)
    -> std::vector<std::string> {
  std::vector<std::string> out;

  return out;
}

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "toyws/http_request.hpp"
#include "toyws/http_response.hpp"

namespace toyws {

class HandlerContext {};

using SyncHandler = void (*)(const HttpRequest&, const HandlerContext&,
                             HttpResponse&);

/**
 * @brief A route used by Router. Can be partial (contains other Routes) or
 * final (endpoint), or both.
 *
 * A (partial/final/both) Route as used by Router. Routes are split at '/'
 * (left-inclusive) and are matched in order they are defined in (see
 * Route::AddRoute).
 */
class Route {
 public:
  /**
   * @param path Full path (including identifier) of this route
   */
  Route(std::string path);

  /**
   * @brief Find first fully matching Route object or nullptr if there is no
   * match.
   *
   * TODO: variable matching & returning the variable.
   */
  auto Match(const std::string& uri) -> Route*;

  /**
   * @brief Find best matching Route object. Will not return nullptr. Thus the
   * "best match" could mean no match at all.
   */
  auto PartialMatch(const std::string& uri) -> Route*;

  /**
   * @brief Check if string starts with this Route's idenitifer.
   */
  auto AppliesTo(const std::string& uri) -> bool {
    return uri.starts_with(idenitifer);
  }

 private:
  std::string fullPath;
  std::string identifier;
  std::vector<Route> subRoutes;
  SyncHandler syncHandler = nullptr;
};

/**
 * @brief Routes incoming requests to handlers.
 */
class Router {
 public:
  /**
   * @brief Add route with synchronous handler
   *
   * Register a route handler that is resolved in a synchronous manner once
   * called upon. This is the correct type if the handling requires no I/O
   * operations and resolve into a HttpResponse without blocking.
   *
   * @remark The order in which routes are defined matter for lookup. E.g.,
   *     router.AddRoute("/users/<name:string>", ...);
   *     router.AddRoute("/users/me", ...);
   * the route "/users/me" will never match because the route before it is
   * applicable.
   */
  auto AddRoute(const std::string& uri, SyncHandler handler) -> void;

  /**
   * @brief Add route with asynchronous handler
   *
   * Register a route handler that is resolved in an asynchronous manner
   * using C++ coroutines once called upon. This is the correct type if the
   * handling requires I/O operations and could not resolve into a HttpResponse
   * without blocking.
   *
   * @remark See remark of Router::AddRoute().
   */
  auto AddRouteCor() -> void {}

  /**
   * @brief Find route from uri. Returns perfect matches only.
   *
   * @return First perfect match (see AddRoute remark), or nullptr if none
   * exist.
   */
  auto FindRoute(const std::string& uri) -> Route*;

  /**
   * @brief Find route from uri. Returns perfect or partial matches.
   *
   * @return First perfect match (see AddRoute remark), if one exists.
   * Otherwise, the first partial match at maximum depth in given sub-tree, if
   * one exists. If there exists no partial match, nullptr is returned.
   */
  auto FindPartialMatchingRoute(const std::string& uri) -> Route*;

 private:
  std::vector<Route> routes;

  auto SplitUri(const std::string& uri) -> std::vector<std::string>;
};

}  // namespace toyws

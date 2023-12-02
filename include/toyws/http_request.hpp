#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include "toyws/error.hpp"
#include "toyws/http_headers_map.hpp"

struct HttpRequestEditor;

namespace toyws {

enum class HttpMethod {
  // NOTE: This violates enum naming conventions of the project. But matches the
  // universal naming convention of HTTP methods.
  GET = 0,
  POST,
  PUT,
  PATCH,
  DELETE,
  OPTIONS,
  HEAD,
  TRACE,
  CONNECT,
};

/**
 * @brief Structured HTTP request.
 *
 * Structured data of a HTTP request. Used to parse a string containing a HTTP
 * request and then access the information in a structured manner.
 */
class HttpRequest {
 public:
  HttpRequest() = default;
  HttpRequest(HttpMethod httpMethod, std::string targetResource)
      : method{httpMethod}, resource{std::move(targetResource)} {}
  HttpRequest(HttpMethod httpMethod, std::string targetResource,
              HeadersMap httpHeaders)
      : method{httpMethod},
        resource{std::move(targetResource)},
        headers{std::move(httpHeaders)} {}
  HttpRequest(HttpMethod httpMethod, std::string targetResource,
              HeadersMap httpHeaders, std::string requestBody)
      : method{httpMethod},
        resource{std::move(targetResource)},
        headers{std::move(httpHeaders)},
        body{std::move(requestBody)} {}
  HttpRequest(HttpMethod httpMethod, std::string targetResource,
              std::string requestBody)
      : method{httpMethod},
        resource{std::move(targetResource)},
        body{std::move(requestBody)} {}

  /**
   * @brief Parse HTTP data from given buffer.
   * @return A truth value if the HTTP read has read a full request. A false
   * value if data is missing. If data is missing, another Read
   * request can be issued to "fill in" the missing pieces, without
   * resupplying the same data.
   */
  auto Read(const char* data, std::size_t length) -> bool;

  /**
   * @brief Write HTTP data.
   * @return Pair (finished, length) indicating if write was partial
   * (finished=false) and how many bytes was put into data.
   */
  auto Write(char* data, std::size_t capacity) -> std::pair<bool, std::size_t>;

  auto Method() const -> HttpMethod { return method; }

  auto Resource() const -> const std::string& { return resource; }

  auto Headers() const -> const HeadersMap& { return headers; }

  auto Body() const -> const std::string& { return body; }

 private:
  HttpMethod method;
  std::string resource;
  HeadersMap headers;
  std::string body;

  friend struct ::HttpRequestEditor;
};

inline auto ParseHttpMethod(const std::string& str) -> HttpMethod {
  if (str == "GET") {
    return HttpMethod::GET;
  } else if (str == "POST") {
    return HttpMethod::POST;
  } else if (str == "PUT") {
    return HttpMethod::PUT;
  } else if (str == "PATCH") {
    return HttpMethod::PATCH;
  } else if (str == "DELETE") {
    return HttpMethod::DELETE;
  } else if (str == "OPTIONS") {
    return HttpMethod::OPTIONS;
  } else if (str == "HEAD") {
    return HttpMethod::HEAD;
  } else if (str == "TRACE") {
    return HttpMethod::TRACE;
  } else if (str == "CONNECT") {
    return HttpMethod::CONNECT;
  } else {
    throw Error(std::string("Invalid HttpMethod: ") + str);
  }
}

inline auto HttpMethodName(HttpMethod method) -> const char* {
  switch (method) {
    case HttpMethod::GET:
      return "GET";
    case HttpMethod::POST:
      return "POST";
    case HttpMethod::PUT:
      return "PUT";
    case HttpMethod::PATCH:
      return "PATCH";
    case HttpMethod::DELETE:
      return "DELETE";
    case HttpMethod::OPTIONS:
      return "OPTIONS";
    case HttpMethod::HEAD:
      return "HEAD";
    case HttpMethod::TRACE:
      return "TRACE";
    case HttpMethod::CONNECT:
      return "CONNECT";
  }

  assert(false && "Unhandled HttpMethod in HttpMethodName()");
}

}  // namespace toyws

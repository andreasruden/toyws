#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

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
  /**
   * @brief Parse HTTP data from given buffer.
   * @return A truth value if the HTTP read has read a full request. A false
   * value if data is missing. If data is missing, another Read request can be
   * issued to "fill in" the missing pieces, without resupplying the same data.
   */
  auto Read(const char* data, std::size_t length) -> bool;

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

}  // namespace toyws

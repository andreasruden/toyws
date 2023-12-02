#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include "toyws/error.hpp"
#include "toyws/http_headers_map.hpp"

struct HttpResponseEditor;

namespace toyws {

enum class HttpStatus {
  // TODO: Add more status codes
  kOk = 200,
  kFound = 302,
  kSeeOther = 303,
  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kUnprocessableContent = 422,
  kTooManyRequests = 429,
  kInternalServerError = 500,
  kHttpVersionNotSupported = 505,
};

class HttpStatusError : public Error {
 public:
  HttpStatusError(HttpStatus statusCode, const std::string& what)
      : Error(what), status{statusCode} {}
  HttpStatusError(HttpStatus statusCode, const char* what)
      : Error(what), status{statusCode} {}

  auto Status() -> HttpStatus { return status; }

 private:
  HttpStatus status;
};

/**
 * @brief Structured HTTP response.
 *
 * Structured data of a HTTP response. Used to fill in HTTP response data, and
 * then write it back to the client.
 */
class HttpResponse {
 public:
  /* Constructors */
  HttpResponse() = default;
  explicit HttpResponse(HttpStatus statusCode) : status{statusCode} {
    FillReason();
  }
  HttpResponse(HttpStatus statusCode, std::string reasonField)
      : status{statusCode}, reason{std::move(reasonField)} {}
  HttpResponse(HttpStatus statusCode, std::string reasonField,
               HeadersMap headersMap)
      : status{statusCode},
        reason{std::move(reasonField)},
        headers{std::move(headersMap)} {}
  HttpResponse(HttpStatus statusCode, std::string reasonField,
               HeadersMap headersMap, std::string content)
      : status{statusCode},
        reason{std::move(reasonField)},
        headers{std::move(headersMap)},
        body{std::move(content)} {}
  HttpResponse(HttpStatus statusCode, HeadersMap headersMap)
      : status{statusCode}, headers{std::move(headersMap)} {
    FillReason();
  }
  HttpResponse(HttpStatus statusCode, HeadersMap headersMap,
               std::string content)
      : status{statusCode},
        headers{std::move(headersMap)},
        body{std::move(content)} {
    FillReason();
  }

  /**
   * @brief Write HTTP data.
   * @return Pair (finished, length) indicating if write was partial
   * (finished=false) and how many bytes was put into data.
   */
  auto Write(char* data, std::size_t capacity) -> std::pair<bool, std::size_t>;

  /**
   * @brief Parse HTTP data from given buffer.
   * @return A truth value if the HTTP read has read a full request. A false
   * value if data is missing. If data is missing, another Read
   * request can be issued to "fill in" the missing pieces, without
   * resupplying the same data.
   */
  auto Read(const char* data, std::size_t length) -> bool;

  auto Status() const -> HttpStatus { return status; }

  auto Reason() const -> const std::string& { return reason; }

  auto Headers() const -> const HeadersMap& { return headers; }

  auto Body() const -> const std::string& { return body; }

 private:
  HttpStatus status;
  std::string reason;
  HeadersMap headers;
  std::string body;

  auto FillReason() -> void {
    if (status == HttpStatus::kOk) {
      reason = "OK";
    } else {
      reason = "ERR";
    }
  }

  friend struct ::HttpResponseEditor;
};

inline auto ParseHttpStatus(const std::string& str) -> HttpStatus {
  auto status = static_cast<HttpStatus>(std::stoi(str));
  switch (status) {
    case HttpStatus::kOk:
      return status;
    case HttpStatus::kFound:
      return status;
    case HttpStatus::kSeeOther:
      return status;
    case HttpStatus::kBadRequest:
      return status;
    case HttpStatus::kUnauthorized:
      return status;
    case HttpStatus::kForbidden:
      return status;
    case HttpStatus::kNotFound:
      return status;
    case HttpStatus::kUnprocessableContent:
      return status;
    case HttpStatus::kTooManyRequests:
      return status;
    case HttpStatus::kInternalServerError:
      return status;
    case HttpStatus::kHttpVersionNotSupported:
      return status;
  }

  throw Error(str + " is not a valid HttpStatus");
}

}  // namespace toyws

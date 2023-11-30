#include <fmt/core.h>

#include <type_traits>

#include "toyws/error.hpp"
#include "toyws/http_request.hpp"
#include "toyws/http_response.hpp"

struct HttpRequestEditor {
  static auto SetMethod(toyws::HttpRequest* request, toyws::HttpMethod method)
      -> void {
    request->method = method;
  }

  static auto SetResource(toyws::HttpRequest* request, std::string&& resource)
      -> void {
    request->resource = std::move(resource);
  }
};

// Fwd Declares:
static auto ReadUntilDelim(const char* data, std::size_t i, std::size_t length,
                           char delim, std::string& buf) -> std::size_t;
static auto ConsumeNewline(const char* data, std::size_t i, std::size_t length)
    -> std::size_t;
static auto ParseRequestLine(const char* data, std::size_t i,
                             std::size_t length, std::string& buf,
                             toyws::HttpRequest* target) -> std::size_t;

static auto ParseHeaderField(const char* data, std::size_t i,
                             std::size_t length, std::string& buf,
                             toyws::HeadersMap* headers) -> std::size_t;

static auto ReadBody(const char* data, std::size_t i, std::size_t length,
                     std::string* body) -> void;
static auto WriteRaw(char* data, std::size_t offset, std::size_t capacity,
                     const char* output, std::size_t length, bool& success)
    -> std::size_t;
static auto WriteRaw(char* data, std::size_t offset, std::size_t capacity,
                     const std::string& output, bool& success) -> std::size_t;

// HttpRequest:
auto toyws::HttpRequest::Read(const char* data, const std::size_t length)
    -> bool {
  std::string buf;
  std::size_t offset = 0;

  offset = ParseRequestLine(data, offset, length, buf, this);

  std::size_t prev;
  do {
    prev = offset;
    offset = ParseHeaderField(data, offset, length, buf, &headers);
  } while (offset != prev);

  offset = ConsumeNewline(data, offset, length);

  ReadBody(data, offset, length, &body);

  // TODO: Implement partial reading, i.e., when buffer has missing request
  // data
  return true;
}

// HttpResponse:
auto toyws::HttpResponse::Write(char* data, const std::size_t capacity)
    -> std::pair<bool, std::size_t> {
  std::size_t i = 0;
  bool success;

  // TODO: Handle running out of buffer capacity (partial writes)

  // Status Line: Http-Version SP Status SP Reason CRLF
  i = WriteRaw(data, i, capacity, "HTTP/1.1 ", 9, success);
  // TODO: Use std::to_underlying from C++23
  i = WriteRaw(data, i, capacity, std::to_string(static_cast<int>(status)),
               success);
  i = WriteRaw(data, i, capacity, " ", 1, success);
  i = WriteRaw(data, i, capacity, reason, success);
  i = WriteRaw(data, i, capacity, "\r\n", 2, success);

  // Header Fields: Key: SP Value CRLF
  for (const auto& header : headers) {
    i = WriteRaw(data, i, capacity, header.first, success);
    i = WriteRaw(data, i, capacity, ": ", 2, success);
    i = WriteRaw(data, i, capacity, header.second, success);
    i = WriteRaw(data, i, capacity, "\r\n", 2, success);
  }

  // CRLF to seperate header & body
  i = WriteRaw(data, i, capacity, "\r\n", 2, success);

  // TODO: Write body + content length

  return std::make_pair(true, i);  // TODO
}

// Shared Implementation:

auto ReadUntilDelim(const char* data, std::size_t i, const std::size_t length,
                    const char delim, std::string& buf) -> std::size_t {
  for (; i < length; ++i) {
    char c = data[i];
    if (c == delim) {
      break;
    } else if (c == '\r' || c == '\n') {
      throw toyws::Error(fmt::format(
          "HttpRequest::ReadUntilDelim: Newline before expected delimiter {}",
          delim));
    } else {
      buf += c;
    }
  }

  return i;
}

auto ConsumeNewline(const char* data, std::size_t i, std::size_t length)
    -> std::size_t {
  if (i < length + 1 && data[i] == '\r' && data[i + 1] == '\n') {
    return i + 2;
  }
  throw toyws::Error("HttpRequest: Expected newline");
}

auto ParseRequestLine(const char* data, std::size_t i, const std::size_t length,
                      std::string& buf, toyws::HttpRequest* target)
    -> std::size_t {
  buf.reserve(8);
  i = ReadUntilDelim(data, i, length, ' ', buf) + 1;
  HttpRequestEditor::SetMethod(target, toyws::ParseHttpMethod(buf));
  buf.clear();

  buf.reserve(64);
  i = ReadUntilDelim(data, i, length, ' ', buf) + 1;
  HttpRequestEditor::SetResource(target, std::move(buf));
  buf.clear();

  buf.reserve(9);
  i = ReadUntilDelim(data, i, length, '\r', buf);
  if (buf != "HTTP/1.1") {
    throw toyws::Error(fmt::format("Expected version HTTP/1.1, given {}", buf));
  }
  buf.clear();

  i = ConsumeNewline(data, i, length);

  return i;
}

auto ParseHeaderField(const char* data, std::size_t i, const std::size_t length,
                      std::string& buf, toyws::HeadersMap* headers)
    -> std::size_t {
  if (data[i] == '\r') {
    return i;
  }

  // Read "Key:"
  buf.reserve(32);
  i = ReadUntilDelim(data, i, length, ':', buf) + 1;
  std::string key{std::move(buf)};
  buf.clear();
  if (headers->contains(key)) {
    throw toyws::Error(
        fmt::format("Header {} stated twice in HTTP request", key));
  }

  // Skip whitespace
  if (data[i] == ' ') {
    ++i;
  }

  // Read rest as header value
  buf.reserve(128);
  i = ReadUntilDelim(data, i, length, '\r', buf);
  headers->emplace(std::move(key), std::move(buf));
  buf.clear();

  // TODO: Skip optional whitespace at end of value?

  i = ConsumeNewline(data, i, length);

  return i;
}

auto ReadBody(const char* data, const std::size_t i, const std::size_t length,
              std::string* body) -> void {
  body->assign(data + i, length - i);
}

auto WriteRaw(char* data, std::size_t offset, std::size_t capacity,
              const char* output, std::size_t length, bool& success)
    -> std::size_t {
  std::size_t i;
  for (i = 0; i < length && offset + i < capacity; ++i) {
    data[offset + i] = output[i];
  }

  success = i == length;

  return offset + i;
}

auto WriteRaw(char* data, std::size_t offset, const std::size_t capacity,
              const std::string& output, bool& success) -> std::size_t {
  return WriteRaw(data, offset, capacity, output.c_str(), output.size(),
                  success);
}

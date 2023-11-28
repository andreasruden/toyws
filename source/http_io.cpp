#include "toyws/http_io.hpp"

#include <fmt/core.h>

#include "toyws/error.hpp"
#include "toyws/http_request.hpp"

// HttpRequest:
auto toyws::HttpRequest::Read(const char* data, const std::size_t length)
    -> bool {
  HttpIo io{this};
  return io.ReadRequest(data, length);
}

template <typename T>
auto toyws::HttpIo<T>::ReadRequest(const char* data, const std::size_t length)
    -> bool {
  std::string buf;
  std::size_t offset = 0;

  offset = ParseRequestLine(data, offset, length, buf);

  std::size_t prev;
  do {
    prev = offset;
    offset = ParseHeaderField(data, offset, length, buf);
  } while (offset != prev);

  offset = ConsumeNewline(data, offset, length);

  ReadBody(data, offset, length);

  // TODO: Implement partial reading, i.e., when buffer has missing request
  // data
  return true;
}

template <typename T>
auto toyws::HttpIo<T>::ReadUntilDelim(const char* data, std::size_t i,
                                      const std::size_t length,
                                      const char delim, std::string& buf) const
    -> std::size_t {
  for (; i < length; ++i) {
    char c = data[i];
    if (c == delim) {
      break;
    } else if (c == '\r' || c == '\n') {
      throw Error(fmt::format(
          "HttpRequest::ReadUntilDelim: Newline before expected delimiter {}",
          delim));
    } else {
      buf += c;
    }
  }

  return i;
}

template <typename T>
auto toyws::HttpIo<T>::ConsumeNewline(const char* data, std::size_t i,
                                      std::size_t length) -> std::size_t {
  if (i < length + 1 && data[i] == '\r' && data[i + 1] == '\n') {
    return i + 2;
  }
  throw Error("HttpRequest: Expected newline");
}

template <typename T>
auto toyws::HttpIo<T>::ParseRequestLine(const char* data, std::size_t i,
                                        const std::size_t length,
                                        std::string& buf) -> std::size_t {
  buf.reserve(8);
  i = ReadUntilDelim(data, i, length, ' ', buf) + 1;
  target->method = ParseHttpMethod(buf);
  buf.clear();

  buf.reserve(64);
  i = ReadUntilDelim(data, i, length, ' ', buf) + 1;
  target->resource = std::move(buf);
  buf.clear();

  buf.reserve(9);
  i = ReadUntilDelim(data, i, length, '\r', buf);
  if (buf != "HTTP/1.1") {
    throw Error(fmt::format("Expected version HTTP/1.1, given {}", buf));
  }
  buf.clear();

  i = ConsumeNewline(data, i, length);

  return i;
}

template <typename T>
auto toyws::HttpIo<T>::ParseHeaderField(const char* data, std::size_t i,
                                        const std::size_t length,
                                        std::string& buf) -> std::size_t {
  if (data[i] == '\r') {
    return i;
  }

  // Read "Key:"
  buf.reserve(32);
  i = ReadUntilDelim(data, i, length, ':', buf) + 1;
  std::string key{std::move(buf)};
  buf.clear();
  if (target->headers.contains(key)) {
    throw Error(fmt::format("Header {} stated twice in HTTP request", key));
  }

  // Skip whitespace
  if (data[i] == ' ') {
    ++i;
  }

  // Read rest as header value
  buf.reserve(128);
  i = ReadUntilDelim(data, i, length, '\r', buf);
  target->headers.emplace(std::move(key), std::move(buf));
  buf.clear();

  // TODO: Skip optional whitespace at end of value?

  i = ConsumeNewline(data, i, length);

  return i;
}

template <typename T>
auto toyws::HttpIo<T>::ReadBody(const char* data, const std::size_t i,
                                const std::size_t length) -> void {
  target->body.assign(data + i, length - i);
}

// HttpIo should only be templetable on Request & Response.
// Explicitly instantiate them:
namespace toyws {
template class HttpIo<HttpRequest>;
}

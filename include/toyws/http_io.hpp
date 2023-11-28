#pragma once

#include <cstddef>
#include <string>

namespace toyws {

class HttpRequest;

template <typename T>
class HttpIo {
 public:
  explicit HttpIo(T* targetObj) : target{targetObj} {}

  /**
   * @brief Parse HTTP data from given buffer.
   * @return A truth value if the HTTP read has read a full request. A false
   * value if data is missing. If data is missing, another Read request can be
   * issued to "fill in" the missing pieces, without resupplying the same data.
   */
  auto ReadRequest(const char* data, std::size_t length) -> bool;

 private:
  T* target;

  auto ReadUntilDelim(const char* data, std::size_t i, std::size_t length,
                      char delim, std::string& buf) const -> std::size_t;

  auto ConsumeNewline(const char* data, std::size_t i, std::size_t length)
      -> std::size_t;

  auto ParseRequestLine(const char* data, std::size_t i, std::size_t length,
                        std::string& buf) -> std::size_t;

  auto ParseHeaderField(const char* data, std::size_t i, std::size_t length,
                        std::string& buf) -> std::size_t;

  auto ReadBody(const char* data, std::size_t i, std::size_t length) -> void;
};

}  // namespace toyws

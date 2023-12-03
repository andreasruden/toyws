#pragma once

#include <vector>

#include "toyws/toyws_export.hpp"

inline constexpr int kBufferSize = 2048;

namespace toyws {

/**
 * @brief Represents a connection to a client (including before it is accepted)
 * with current state & a read/write buffer.
 */
class TOYWS_EXPORT Client {
 public:
  enum class States { kAccept = 0, kRead, kWrite, kFinished };

  Client() { buffer.resize(kBufferSize); }

  auto State() const -> States { return state; }
  auto SetState(States newState) -> void { state = newState; }

  auto Socket() const -> int { return clientFd; }
  auto SetSocket(int fd) -> void { clientFd = fd; }

  auto IoServiceSlot() const -> int { return ioServiceSlot; }
  auto SetIoServiceSlot(int slot) -> void { ioServiceSlot = slot; }

  /**
   * @brief Buffer for network reading/writing.
   */
  auto Buffer() -> std::vector<char>& { return buffer; }

  /**
   * @brief How much content is in the buffer. Bytes in range
   * Buffer()[0..BufferContentSize] indicates the content that was read/written.
   * It does not necessarily correspond to the buffer size.
   */
  auto BufferContentSize() const -> std::size_t { return bufferContentSize; }
  auto SetBufferContentSize(std::size_t newSize) -> void {
    bufferContentSize = newSize;
  }

 private:
  States state = States::kAccept;
  int clientFd = 0;
  int ioServiceSlot = -1;
  std::vector<char> buffer;
  std::size_t bufferContentSize = 0;
};

}  // namespace toyws

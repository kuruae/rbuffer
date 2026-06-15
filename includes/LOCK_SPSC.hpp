#include <array>
#include <cstddef>
#include <mutex>
#include <utility>

template <typename T, size_t Capacity> class LOCK_SPSC {
private:
  std::array<T, Capacity> buffer_;
  size_t head_{0};
  size_t tail_{0};
  std::mutex mutex_;

public:
  bool push(T &&item) {
    std::lock_guard<std::mutex> lock(mutex_);

    if ((tail_ - head_) == Capacity)
      return false;

    buffer_[tail_ % Capacity] = std::move(item);

    tail_++;
    return true;
  }

  bool pop(T &item) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (tail_ == head_)
      return false;

    item = std::move(buffer_[head_ % Capacity]);

    head_++;
    return true;
  }
};

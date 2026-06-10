#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t Capacity> class SPCS {
private:
  std::array<T, Capacity> buffer_;

  alignas(64) std::atomic_size_t head_{0};
  alignas(64) std::atomic_size_t tail_{0};

public:
  bool push(T &&item) {
    const size_t t = tail_.load(std::memory_order_acquire);
    const size_t h = head_.load(std::memory_order_acquire);

    if ((t - h) == Capacity)
      return false;

    buffer_[t % Capacity] = std::move(item);

    tail_.store(t + 1, std::memory_order_release);
    return true;
  }

  bool pop(T &item) {
    const size_t h = head_.load(std::memory_order_acquire);
    const size_t t = tail_.load(std::memory_order_acquire);

    if (t == h)
      return false;

    item = std::move(buffer_[h % Capacity]);

    head_.store(h + 1, std::memory_order_release);
    return true;
  }
};

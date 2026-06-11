#include <array>
#include <atomic>
#include <cstddef>
#include <new>

template <typename T, size_t Capacity> class SPSC {
private:
  alignas(std::hardware_destructive_interference_size)
      std::array<T, Capacity> buffer_;

  alignas(std::hardware_destructive_interference_size) std::atomic_size_t head_{
      0};

  alignas(std::hardware_destructive_interference_size) std::atomic_size_t tail_{
      0};

public:
  bool push(T &&item) {
    const size_t t = tail_.load(std::memory_order_relaxed);
    const size_t h = head_.load(std::memory_order_acquire);

    if ((t - h) == Capacity)
      return false;

    buffer_[t % Capacity] = std::move(item);

    tail_.store(t + 1, std::memory_order_release);
    return true;
  }

  bool pop(T &item) {
    const size_t h = head_.load(std::memory_order_relaxed);
    const size_t t = tail_.load(std::memory_order_acquire);

    if (t == h)
      return false;

    item = std::move(buffer_[h % Capacity]);

    head_.store(h + 1, std::memory_order_release);
    return true;
  }
};

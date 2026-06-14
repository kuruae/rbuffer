#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <new>

template <typename T, size_t Capacity> class SPSC {
  static_assert(std::has_single_bit(Capacity),
                "Capacity must be a power of two");

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

    buffer_[t & (Capacity - 1)] = std::move(item);

    tail_.store(t + 1, std::memory_order_release);
    return true;
  }

  bool pop(T &item) {
    const size_t h = head_.load(std::memory_order_relaxed);
    const size_t t = tail_.load(std::memory_order_acquire);

    if (t == h)
      return false;

    item = std::move(buffer_[h & (Capacity - 1)]);

    head_.store(h + 1, std::memory_order_release);
    return true;
  }
};

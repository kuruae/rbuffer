
#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t Capacity> class NaiveSpsc {
private:
  std::array<T, Capacity> buffer_;
  std::atomic_size_t head_{0};
  std::atomic_size_t tail_{0};

public:
  bool push(T &&item) {
    const size_t t = tail_.load();
    const size_t h = head_.load();

    if ((t - h) == Capacity)
      return false;

    buffer_[t % Capacity] = std::move(item);

    tail_.store(t + 1);
    return true;
  }

  bool pop(T &item) {
    const size_t h = head_.load();
    const size_t t = tail_.load();

    if (t == h)
      return false;

    item = std::move(buffer_[h % Capacity]);

    head_.store(h + 1);
    return true;
  }
};

#include <boost/lockfree/spsc_queue.hpp>

template <typename T, size_t Capacity> class BoostSpsc {
private:
  boost::lockfree::spsc_queue<T, boost::lockfree::capacity<Capacity>> queue_;

public:
  [[nodiscard]] inline bool push(T &&item) {
    return queue_.push(std::move(item));
  }

  inline bool pop(T &item) { return queue_.pop(item); }
};

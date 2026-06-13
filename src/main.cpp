#include "../includes/FS_SPSC.hpp"
#include "../includes/MTX_SPSC.hpp"
#include "../includes/SPSC.hpp"
#include <cassert>
#include <cstddef>
#include <print>
#include <thread>

static constexpr size_t n{1000000};

template <typename Qtype> void runTest() {
  Qtype queue;
  size_t produced_sum{};
  size_t consumed_sum{};

  std::thread producer([&] {
    for (size_t i{}; i < n; i++) {
      produced_sum += i;
      while (!queue.push(static_cast<size_t>(i)))
        ;
    }
  });

  std::thread consumer([&] {
    size_t received{};
    while (received < n) {
      size_t val{};
      if (queue.pop(val)) {
        consumed_sum += val;
        received++;
      }
    }
  });

  producer.join();
  consumer.join();

  assert(produced_sum == consumed_sum);
}

/**
 * MAIN SPSC:
 * Main implementation, with all possible optimizations
 *
 * FS SPSC:
 * Atomics are false shared (share the same cache line)
 * No specified cache ordering (default order)
 *
 * MTX SPSC:
 * Lock-based queue
 */
int main() {
  runTest<SPSC<size_t, 1024>>();
  std::println("[MAIN SPSC]: Correctness OK: {} messages\n", n);

  runTest<FS_SPSC<size_t, 1024>>();
  std::println("[FS SPSC]: Correctness OK: {} messages\n ", n);

  runTest<MTX_SPSC<size_t, 1024>>();
  std::println("[MUTEX SPSC]: Correctness OK: {} messages\n ", n);

  return 0;
}

#include "../includes/BOOST_SPSC.hpp"
#include "../includes/FS_SPSC.hpp"
#include "../includes/MTX_SPSC.hpp"
#include "../includes/SPSC.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <print>
#include <thread>
#include <vector>

static constexpr size_t N{10'100'000};

inline void pin_thread(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

inline void inline_pause() { __builtin_ia32_pause(); }

[[nodiscard]]
inline uint64_t rdtsc_start() {
  uint32_t lo, hi;
  __asm__ volatile("lfence\n\trdtsc" : "=a"(lo), "=d"(hi));
  return static_cast<uint64_t>(hi) << 32 | lo;
}

[[nodiscard]]
inline uint64_t rdtsc_end() {
  uint32_t lo, hi, aux;
  __asm__ volatile("rdtscp\n\tlfence" : "=a"(lo), "=d"(hi), "=c"(aux));
  return static_cast<uint64_t>(hi) << 32 | lo;
}

template <typename Qtype> void runTest(const std::string &test_name) {
  Qtype queue;
  std::vector<uint64_t> latencies(N, 0);
  std::atomic<bool> start_signal{false};

  std::jthread producer([&] {
    pin_thread(2);

    while (!start_signal.load(std::memory_order_acquire)) {
      inline_pause();
    }

    for (size_t i{}; i < N; i++) {
      uint64_t timestamp{rdtsc_start()};

      while (!queue.push(std::move(timestamp))) {
        inline_pause();
        timestamp = rdtsc_start();
      }

      uint64_t pace_until = rdtsc_start() + 3000;
      while (rdtsc_start() < pace_until) {
        inline_pause();
      }
    }
  });

  std::jthread consumer([&] {
    pin_thread(4);

    start_signal.store(true, std::memory_order_release);

    for (size_t i = 0; i < N; i++) {
      uint64_t sent{};
      while (!queue.pop(sent)) {
        inline_pause();
      }
      uint64_t end_time = rdtsc_end();

      latencies[i] = (end_time > sent) ? (end_time - sent) : 0;
    }
  });

  producer.join();
  consumer.join();

  std::ranges::sort(latencies);

  std::println("[{}]", test_name);
  std::println("  p50  :   {} cycles", latencies[N * 50 / 100]);
  std::println("  p99  :   {} cycles", latencies[N * 99 / 100]);
  std::println("  p999 :   {} cycles", latencies[N * 999 / 1000]);
  std::println("-----------------");
}

/**
 * MAIN SPSC:
 * Main implementation, with all possible optimizations
 *
 * BOOST SPSC:
 * Boost library implementation
 *
 * FS SPSC:
 * Atomics are false shared (share the same cache line)
 * No specified cache ordering (default order)
 *
 * MTX SPSC:
 * Lock-based queue
 */
int main() {
  pin_thread(0);

  runTest<SPSC<size_t, 1024>>("MAIN SPSC");
  runTest<BOOST_SPSC<size_t, 1024>>("BOOST SPSC");
  runTest<FS_SPSC<size_t, 1024>>("FS SPSC");
  runTest<MTX_SPSC<size_t, 1024>>("MUTEX SPSC");

  return 0;
}

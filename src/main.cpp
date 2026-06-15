#include "../includes/BOOST_SPSC.hpp"
#include "../includes/FS_SPSC.hpp"
#include "../includes/LOCK_SPSC.hpp"
#include "../includes/SPSC.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <print>
#include <thread>
#include <vector>

static constexpr size_t N{10'100'000};
static constexpr size_t WARMUP{100'000};

inline void pin_thread(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

inline void inline_pause() noexcept { __builtin_ia32_pause(); }

[[nodiscard]]
inline uint64_t rdtsc_start() noexcept {
  uint32_t lo, hi;
  __asm__ volatile("lfence\n\trdtsc" : "=a"(lo), "=d"(hi));
  return static_cast<uint64_t>(hi) << 32 | lo;
}

[[nodiscard]]
inline uint64_t rdtsc_end() noexcept {
  uint32_t lo, hi, aux;
  __asm__ volatile("rdtscp\n\tlfence" : "=a"(lo), "=d"(hi), "=c"(aux));
  return static_cast<uint64_t>(hi) << 32 | lo;
}

template <typename Qtype> void run_test(const std::string_view name) {
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

        // re-stamp: measure push round-trip, not wait time
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

  // already joins when it goes out of scope btw but printing
  // the results require an early join
  producer.join();
  consumer.join();

  auto measured = std::span(latencies).subspan(WARMUP);
  std::ranges::sort(measured);

  const auto p = [&](double pct) -> uint64_t {
    const size_t idx = static_cast<size_t>(pct / 100.0 * measured.size());
    return measured[std::min(idx, measured.size() - 1)];
  };

  std::println("[{}]", name);
  std::println("  p50  : {:>8} cycles", p(50.0));
  std::println("  p99  : {:>8} cycles", p(99.0));
  std::println("  p999 : {:>8} cycles", p(99.9));
  std::println("-----------------");
}

/**
 * MAIN SPSC:
 * Main implementation, with all possible optimizations i could personally find
 *
 * BOOST SPSC:
 * Boost library implementation, wrapped around a reusable struct
 *
 * FS SPSC:
 * Atomics are false shared (share the same cache line)
 * No specified cache ordering (default order)
 *
 * LOCK SPSC:
 * Lock-based queue using mutexes
 */
int main() {
  pin_thread(0);

  run_test<SPSC<size_t, 1024>>("My SPSC");
  run_test<BOOST_SPSC<size_t, 1024>>("Boost SPSC");
  run_test<FS_SPSC<size_t, 1024>>("False sharing SPSC");
  run_test<LOCK_SPSC<size_t, 1024>>("Lock-based SPSC");
}

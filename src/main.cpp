#include "../includes/SPSC.hpp"
#include <cassert>
#include <cstddef>
#include <print>
#include <thread>

int main() {
  constexpr size_t n{1000000};
  SPSC<size_t, 1024> queue;

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
  std::println("Correctness OK: {} messages", n);
  return 0;
}

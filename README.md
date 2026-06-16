# SPSC Ring Buffer Benchmarks (`rbuffer`)

A study of single-producer single-consumer ring buffers in C++. This project focuses on benchmarking different SPSC queue designs, focusing particularly on lock-free concurrency, memory ordering, and cache hardware awareness (like avoiding false sharing). 

To benchmark how well these implementations handle cross-core communication, this repository measures the push-to-pop latency in exact CPU cycles.

## Implementations

The project compares four different SPSC implementations:

1. **My SPSC (`SPSC.hpp`)**: A custom, optimized lock-free ring buffer. It utilizes strict memory ordering constraints (`std::memory_order_acquire`/`release`) and explicit cache line padding to prevent false sharing between producer and consumer atomic variables.
2. **Boost SPSC (`BOOST_SPSC.hpp`)**: A wrapper around the widely used `boost::lockfree::spsc_queue`, serving as our baseline.
3. **False sharing SPSC (`FS_SPSC.hpp`)**: A naive lock-free implementation. It uses the default memory ordering (sequential consistency) and does *not* separate atomic indices into different cache lines, causing the CPU cores to constantly invalidate each other's cache (cache line ping-pong).
4. **Lock-based SPSC (`LOCK_SPSC.hpp`)**: A traditional queue protected by a standard `std::mutex`.

## Testing Methodology

* **Hardware Timestamps**: Latency is measured directly in CPU cycles using the `rdtsc` and `rdtscp` instruction intrinsics, wrapping the moment an item is generated to the moment it is received by the consumer.
* **Core Pinning**: The producer and consumer threads are pinned to separate, specific CPU cores (Core 2 and Core 4). This forces data to travel across the CPU interconnect, stressing the cache coherency protocol rather than relying on shared L1/L2 caches.
* **Paced Workload**: The producer paces itself (waiting ~3000 cycles between pushes) to simulate a realistic workload, avoiding a scenario where the queue is always completely full.
* **Warmup Phase**: To avoid skewed numbers from cold starts, the first 100,000 pushes are discarded. The percentiles are calculated over the remaining 99,000,000 samples.

## Performance

Here is a recent benchmark sample on my machine measuring latency percentiles (lower is better):

```text
[My SPSC]
  p50  :      517 cycles
  p99  :      582 cycles
  p999 :      667 cycles
-----------------
[Boost SPSC]
  p50  :      433 cycles
  p99  :      573 cycles
  p999 :      655 cycles
-----------------
[False sharing SPSC]
  p50  :      611 cycles
  p99  :      703 cycles
  p999 :      845 cycles
-----------------
[Lock-based SPSC]
  p50  :     1501 cycles
  p99  :     2566 cycles
  p999 :     2803 cycles
-----------------
```

![Benchmark Results](benchmark_results.png)

### Observations

1. **Locks aren't even competitive**: As expected, the `Lock-based` approach is a lot slower, OS-level synchronization and mutex contention cost around ~3x more cycles at the median (p50) and over ~4x more in the 99.9th percentile compared to lock-free implementations.
2. **False sharing**: The difference between `My SPSC` and the `False sharing SPSC` showcases that placing atomic counters too close to each other in memory degrades performance by ~20% overall and causes much worse tail latencies.
3. **Competitive with the Standard**: `My SPSC` performs well against the `Boost SPSC`. While Boost gets an edge in the median (~433 vs ~517 cycles), `My SPSC` catches right up at the tail ends, showing almost identical p99 and p999 consistency.

## Building and Running

You will need a compiler that supports C++23 and Boost installed on your system.
I also assume this only work on i386 systems since I have no idea about how the ``rdtsc`` registeries behave on other architectures.

```bash
make
./rbuf
```

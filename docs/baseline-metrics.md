# Baseline Performance Metrics

**Date:** 29-12-2025
**Status:** Baseline (No Caching, Direct Disk I/O)

## 1. Test Environment

* **OS:** macOS (arm64)
* **Compiler:** Apple Clang / C++20
* **Storage Media:** SSD (Internal Mac Storage)
* **Hardware:** Macbook Pro M3, 36GB

## 2. Benchmark Configuration

The following Parameters were used to establish the control group. All reads were performed using `std::fstream` with no application-level buffering.

* **Page Size:** 4KB (4096 bytes)
* **Dataset Size:** 10,000 Pages (~40 MB)
* **Read Batch Size:** 1,000 Pages per iteration
* **Iterations:** 10 (excluding warm-ups)
* **I/O Method:** Synchronous `seekg` + `read`

## Results

*Values represent the avergae across 10 iterations.*

| Metric | Sequential Scan | Random Access |
| :--- | :--- | :--- |
| **Total Time (Avg)** | 1.905 ms | 1.991 ms |
| **Latency per Page** | 1.905 µs | 1.991 µs |
| **Throughput** | 524937.773 pages/sec | 502226.546 pages/sec |
| **Bandwidth** | 2050.538 MB/s | 1961.822 MB/s |

## 4. Analysis and Observations

* **Sequential Access:** The OS filesytem cache and hardware prefetching likely aided performance here, resulting in lower latency even without application buffering.
* **Random Access:** Performance drops significantly (typically 10x-50x slower) due to the lack of prefecthing and the overhead of seeking to non-continous file offsets.
* **Bottleneck:** The primary bottleneck is the syscall overhead of `read()` for every single 4KB block and the physical limitations of storage I/O for random patterns.

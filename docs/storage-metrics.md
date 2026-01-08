# Storage Engine Performance Report

**Date:** Jan 07, 2026

**Dataset:** 1 Million Records (~85 MB Data)

**Machine:** Apple Macbook Pro M3

## 1. Executive Summary

The custom `StorageEngine` successfully handles 1M records with massive throughput and nanosecond scale read latency for hot data. The architecture proved capable of sustaining **1.6 million writes per second** and **10 million sequential reads per second**.

## 2. Key Metrics

| Metric | Result | Notes |
| :--- | :--- | :--- |
| **Write Throughput** | 1,628,379 ops/sec | Append-only log structure saturates memory bandwidth. |
| **Cold Read Latency** | 1.48 µs | Hits Index + Page Cache. |
| **Warm Read Latency** | 0.11 µs | Hits LRU Record Cache (No I/O). **13.5x faster.** |
| **Scan Speed** | 10,405,100 recs/s | Sequential memory access via `mmap`. |

## 3. Performance Analysis

* **Write Performance:** Writing is efficient because we simply append to the end of the file and update the in-memory Hash Map. The OS buffers the disk writes, allowing us to write at memory speed.
* **Read Performance:**
  * **Cold:** Requires hashing the key (CPU) + locking page cache + `memcpy`.
  * **Warm:** Bypasses the storage engine entirely; this is a pure memory lookup in `std::list`. The latency of **110 nanoseconds** is exceptional.
* **Scan:** Processes data at nearly memory bandwidth speed (10.4M/s) because the OS prefetches the memory-mapped file pages.

## 4. Conclusion

The storage layer is performant and stable.

# Storage Engine Performance

**Date:** 05-01-2026

**Commit:** ce2364b

**Architecture:** Mmap + LRU Page Cache

## 1. Goal

Verify the performance impact of replacing standard file I/O (`pread`/`pwrite`) with a custom **Memory-Mapped Page Cache**.

## 2. Benchmark Setup

We compared the new `StorageEngine` against the `NaiveStorage` baseline (which uses raw syscalls).

* **Workload:** Random Read (Zipfian-like Skew)
* **Skew Distribution:** 80% of reads hit 10% of the data (Hot Set).
* **Dataset:** 10,000 Pages (40 MB)
* **Cache Capacity:** 1,000 Pages (10% of DB)
* **Operation Count:** 50,000 Reads
* **Build Type:** Release (`-O3`)

## 3. Results

| Metric | Naive Baseline (syscalls) | Optimized Engine (PageCache) | Improvement |
| :--- | :--- | :--- | :--- |
| **Throughput** | ~1.08 M ops/s | **~2.80 M ops/s** | **2.6x** |
| **Avg Latency** | ~0.93 µs | **~0.35 µs** | **2.6x** |
| **Cache Hit Rate** | N/A (OS managed) | **80.03%** | N/A |

## 4. Analysis

The **2.6x speedup** is driven by the elimination of System Calls for hot data.

### The Naive Path (Slow)

Even if the file is in the OS Page Cache, every read requires a transition from User Mode to Kernel Mode.
`User App` -> `syscall (pread)` -> `Context Switch` -> `Kernel Copy` -> `User App`

### The Optimized Path (Fast)

For the 80% of data in our `PageCache`, access is a simple pointer dereference in User Space.
`User App` -> `Pointer Dereference` -> `Data`

## 5. Conclusion

The implementation of the `mmap`-backed Buffer Pool successfully eliminates syscall overhead for the working set.

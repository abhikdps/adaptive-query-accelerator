# LRU Cache Performance

**Date:** Jan 07, 2026

**Commit:** a2972d4

**Architecture:** Tiered Caching (LRU Record Cache + Page Cache)

## 1. Goal

Measure the latency reduction provided by the **Hot Record Cache** (LRU) for frequently accessed keys.

## 2. Benchmark Setup

* **Workload:** Repeated access to the same key ("Hot Key").
* **Comparison:**
    1. **Cold Read:** First access (Hits Index + Page Cache).
    2. **Warm Read:** Second access (Hits LRU Record Cache).

## 3. Results

| Method | Latency (ms) | Latency (µs) | Improvement |
| :--- | :--- | :--- | :--- |
| **Cold Read (Index)** | 0.00225 ms | 2.25 µs | Baseline |
| **Warm Read (LRU)** | 0.00017 ms | 0.17 µs | 13x Faster |

## 4. Conclusion

The LRU Cache provides sub-microsecond access times (167 nanoseconds) for hot data, bypassing the storage engine logic entirely.

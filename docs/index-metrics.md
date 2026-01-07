# Indexing Performance

**Date:** Jan 07, 2026

**Commit:** 6d4bda4

**Architecture:** In-Memory Hash Index + Memory Mapped Records

## 1. Goal

Compare the read latency of **O(1) Direct Lookup** (via Hash Index) versus **O(N) Full Table Scan** (via Linear Iterator).

## 2. Benchmark Setup

* **Records:** 50,000 (Key="key_X", Value="value_data_X")
* **Data Volume:** ~2 MB (Fits in L3 Cache)
* **Access Pattern:** Random Read of a specific key near the end of the file.
* **Engine:** `StorageEngine` with mapped file I/O.

## 3. Results

| Method | Time Cost | Complexity | Speedup |
| :--- | :--- | :--- | :--- |
| **Full Table Scan** | 5.66 ms | O(N) | 1x (Baseline) |
| **Hash Index Lookup** | 0.003 ms | O(1) | 1834x |

## 4. Analysis

The **1800x speedup** is achieved because the Index allows us to jump directly to the correct Page and Slot offset.

* **Scan:** Must parse headers and iterate 50,000 slots to find a match.
* **Index:** Hashes the key, finds the `RecordID` map entry, and performs 1 pointer calculation.

## 5. Conclusion

The in-memory index functionality satisfies the high-performance read requirements for the Key-Value store phase.

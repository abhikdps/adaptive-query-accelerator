# Adaptive Query Accelerator (AQA)

[![Build Status](https://github.com/abhikdps/adaptive-query-accelerator/actions/workflows/cmake.yml/badge.svg)](https://github.com/abhikdps/adaptive-query-accelerator/actions/workflows/cmake.yml)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

An Adaptive Query Accelerator in C++ that dynamically optimizes data access paths using learned caching and prefetching strategies.

AQA is currently a high-performance, **persistent Key-Value Store** built from scratch. It features a custom storage engine leveraging memory-mapped I/O (`mmap`), a slotted-page data layout, tiered caching (Page + Record), and thread-safe concurrency.

## Architecture

The system is designed with a strict hierarchy of ownership, separating the logical data view (Database) from the physical storage (Engine).

```mermaid
graph TD
    User["User / Application"] -->|put/get/scan| DB["Database Manager"]
    DB -->|Read Lock| LRU["LRU Cache (Hot Records)"]
    DB -->|Write Lock| Index["Hash Index (Key -> RecordID)"]

    subgraph "Storage Engine"
        Index -->|RecordID| Reader["Storage Reader"]
        DB -->|Append| Writer["Storage Writer"]
        Reader -->|PageID| Engine["Storage Engine"]
        Writer -->|Bytes| Engine
    end

    subgraph "Storage Layer"
        Engine -->|read/write| PageCache["Page Cache (buffer pool)"]
        PageCache <-->|mmap/sync| Disk["Physical Disk"]
    end
```

### Core Components

* **Database (The Orchestrator):** Manages the lifecycle of components and enforces concurrency rules using **Reader-Writer Locks** (`std::shared_mutex`). This allows for high-throughput parallel reads while ensuring atomic writes.
* **Indexing:** An in-memory **Hash Index** (`std::unordered_map`) maps keys to their physical disk location (PageID + SlotID), providing **O(1)** lookup complexity.
* **Tiered Caching:**
  * **L1 (Record Cache):** An LRU cache stores deserialized "hot" records, bypassing the storage engine entirely for frequently accessed data (~110ns latency).
  * **L2 (Page Cache):** A custom in-process buffer pool (not the OS page cache) holds a fixed number of 4KB pages in RAM. It uses an LRU-style eviction by default and supports pluggable policies (e.g. scan-resistant, LFU, hint-aware). The backing file is memory-mapped (`mmap`) for zero-copy reads.
* **Storage Engine:** Handles persistent storage. Reads are performed via pointer arithmetic (zero syscalls for cached pages), and writes use an append-only strategy for maximum throughput.
* **Slotted Pages:** Data is organized into 4KB pages with a slot directory, allowing for efficient record management, variable-length records, and internal fragmentation control.

## Performance Benchmarks

| Metric | Result | Analysis |
| :--- | :--- | :--- |
| **Write Throughput** | **1.62M ops/sec** | Append-only design saturates memory bandwidth; OS handles async disk flush. |
| **Scan Speed** | **10.4M recs/sec** | Zero-copy access via `mmap`. |
| **Cold Read Latency** | **1.48 µs** | Requires hashing, page lookup, and slot parsing. |
| **Warm Read Latency** | **0.11 µs** | **13.5x faster** than cold read. Hits L1 Record Cache, bypassing storage layer entirely. |

***Note:** Benchmarks run on Apple Macbook Pro M3 (1M Records, ~85MB Dataset)*

## Design Decisions

* **Why mmap?** Avoids double-buffering (copying data from Kernel space to User space), reducing CPU overhead for reads.
* **Why Slotted Pages?** Decouples record logic from physical offsets. This allows us to defragment or reorder records within a page without breaking external pointers (RecordIDs).
* **Why Reader-Writer Locks?** Database workloads are typically read-heavy (90/10 split). Blocking readers for every write would be inefficient; `std::shared_mutex` allows multiple concurrent readers.

## Build & Run

The project uses CMake and requires a **C++20** compliant compiler.

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build
```

### Running Benchmarks

To reproduce the performance numbers, run the storage benchmark suite:

```bash
# Runs the 1M record stress test (Write, Cold Read, Warm Read, Scan)
./build/benchmarks/storage_benchmark
```

To compare eviction policies under a mixed workload (working set + scan + working set):

```bash
./build/benchmarks/eviction_benchmark
```

## Testing

The codebase follows strict RAII (Resource Acquisition Is Initialization) principles.

* No raw new / delete.
* unique_ptr manages component lifecycles.
* Address Sanitizer (ASAN) compatible for memory safety verification.

To run the unit and integration test suite:

```bash
ctest --test-dir build/ --output-on-failure
```

### Test Coverage

* **AccessObserverTest** — Access observer ring buffer, hit/miss recording, `get_access_count`, and integration with PageCache and Database.
* **PrefetchTest** — Prefetch_page bounds and scan with prefetch (record count).
* **EvictionPolicyTest** — LRU page/record policies; ScanResistant, LFU, ScanResistantThenLfu, HintAware; null observer fallback; single candidate; PageCache with HintAware.
* **WorkloadHintTest** — Workload hint get/set, ScanScope restore (nested and previous), Database::scan sets hint during callback.
* **MappedFileTest** — Persistence and file growth.
* **PageCacheTest** — LRU eviction and cache hits/misses.
* **PinningTest** — Active pages are never evicted (buffer pool safety).
* **IntegrationTest** — Full stack from API to disk.
* **ReaderTest** — Slotted page parser and data integrity.
* **RecoveryTest** — Data persistence, index reconstruction, and Database::scan after restart.
* **ConcurrencyTest** — Thread safety under read/write contention.

### Documentation

| Doc | Description |
| --- | ----------- |
| [eviction-policy.md](eviction-policy.md) | Pluggable eviction (LRU, ScanResistant, LFU, HintAware); testing and benchmark. |
| [workload-hints.md](workload-hints.md) | WorkloadHint (PointLookup/Scan), Database::scan(), ScanScope. |
| [access-observer.md](access-observer.md) | Access observer API and where it is used. |
| [prefetch.md](prefetch.md) | Sequential prefetch during scan. |
| [storage-architecture.md](storage-architecture.md) | Storage engine, page cache, mmap, pinning, eviction. |
| [storage-format.md](storage-format.md) | Slotted-page layout, record format, RID. |

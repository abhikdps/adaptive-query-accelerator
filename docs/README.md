# Adaptive Query Accelerator (AQA)

An Adaptive Query Accelerator in C++ that dynamically optimizes data access paths using learned caching and prefetching strategies.

Currently, AQA features a high-performance **Storage Engine** built from scratch leveraging memory-mapped I/O (`mmap`), a custom LRU Buffer Pool and strict RAII resource management.

## Architecture

The storage layer is designed with a strict hierarchy of ownership and separation of concerns:

```text
StorageEngine (The Facade)
 ├── Owns: MappedFile (The Disk Layer)
 │    └── Manages: OS File Descriptor & mmap region (Zero-copy I/O)
 └── Owns: PageCache  (The Memory Layer)
      ├── Manages: std::vector<RawPage> (Fixed-size Memory Pool)
      ├── Implements: LRU Eviction Policy
      └── Creates: PageHandle (RAII Lease)
           └── Automatically unpins pages on destruction
```

## Core Components

- **MappedFile:** Handles persistent storage using mmap. Reads are performed via pointer arithmetic (zero syscalls for cached pages) and writes are synchronized using msync/fsync.

- **PageCache:** A fixed-size Buffer Pool that manages hot pages in RAM. It uses a Hash Map for O(1) lookups and a Doubly Linked List for O(1) LRU eviction.

- **PageHandle:** A smart pointer wrapper that ensures thread-safe "pinning" of pages. It guarantees pages cannot be evicted while actively used by a query.

## Build

The project uses CMake and requires a **C++20** compliant compiler.

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build
```

***Note:** To enable Address Sanitizer (ASAN) for memory debugging, use -DCMAKE_BUILD_TYPE=Debug.*

## Run

The app CLI tool exposes the storage engine primitives.

1. **Generate Data**

   Create a database file with 10,000 pages (~40MB).

    ```bash
    ./build/src/app generate 10000
    ```

2. **Read a Page**

    Fetch a specific page by ID (this demonstrates the fetch_page -> PageHandle pipeline).

    ```bash
    ./build/src/app read 3
    ```

3. **Benchmarks**

    Measure the latency and throughput of the storage engine.

    **Sequential Scan:** Simulates a table scan. Since we use mmap, the OS readahead aggressively prefetches future pages.

    ```bash
    ./build/src/app benchmark seq 10000
    ```

    **Random Access:** Simulates index lookups. This stresses the PageCache eviction logic and disk seek time.

    ```bash
    ./build/src/app benchmark rnd 10000
    ```

## Testing

The codebase follows strict RAII (Resource Acquisition Is Initialization) principles.

- No raw new / delete.
- Copy constructors are deleted on resource-heavy classes (MappedFile, PageCache) to prevent double-free errors.
- unique_ptr manages component lifecycles.

To run the unit and integration test suite:

```bash
ctest --test-dir build/ --output-on-failure
```

### Test Coverage

- MappedFileTest: Verifies persistence and file growth.
- PageCacheTest: Verifies LRU eviction and cache hits/misses.
- PinningTest: Verifies that active pages are never evicted (Buffer Pool safety).
- IntegrationTest: Verifies the full stack from API to Disk.

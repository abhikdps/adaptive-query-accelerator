# Sequential Scan Prefetch

During a full table scan, the next few pages are loaded into the page cache in the background so that by the time the scan needs them, they are often already present (cache hit), reducing wait on I/O.

## Status

**Implemented.** Prefetch is triggered automatically inside `StorageReader::scan()`; no API change for callers. See [workload-hints.md](workload-hints.md) for how `Database::scan()` sets the workload hint so eviction and prefetch align with the scan.

## How it works

- **`StorageEngine::prefetch_page(page_id)`**  
  Bounds-checks `page_id`, then starts a detached thread that calls `fetch_page(page_id)` and drops the handle. The page is loaded into the page cache and unpinned when the thread finishes. Best-effort: invalid `page_id` is a no-op; exceptions in the thread are ignored.

- **Trigger in `StorageReader::scan()`**  
  After processing each page `pid`, the reader calls `engine_.prefetch_page(pid + 1)` and `engine_.prefetch_page(pid + 2)` when in range (`PREFETCH_LOOKAHEAD = 2`). So while the main thread works on page N, the next one or two pages can be loading in the background.

## Where it is used

| Layer | Usage |
| ----- | ----- |
| **StorageEngine** | `void prefetch_page(uint32_t page_id)` — public API; loads page in a background thread. |
| **StorageReader** | At the end of each page iteration in `scan()`, prefetches `pid+1` and `pid+2` (if within `get_total_pages()`). |

Scans performed via `Database` (e.g. `StorageReader::scan()` used by benchmarks or index rebuild) use the same reader and thus get prefetch automatically.

## Correctness

Prefetch does not change semantics: it only loads pages into the cache earlier. No ordering or visibility guarantees are affected.

## Testing

- **PrefetchTest** (`tests/prefetch_test.cpp`): `prefetch_page` with in-range and out-of-range IDs (no crash); full scan with prefetch returns correct record count.
- Run: `./build/tests/prefetch_test` or `ctest --test-dir build/ -R PrefetchTest`

## Design notes

- **Thread safety:** `PageCache::fetch_page` is mutex-protected; the prefetch thread is safe. The engine must outlive any in-flight prefetch (normal when scan runs to completion).
- **Lookahead:** `PREFETCH_LOOKAHEAD = 2` in `storage_reader.cpp`; can be tuned or made configurable later.

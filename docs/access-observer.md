# Access Observer

The **Access Observer** records recent page-level access patterns (page ID and cache hit/miss) in a fixed-size ring buffer. It is the data source for future adaptive eviction and prefetching: eviction policies and prefetchers can read the last N accesses to decide what to keep or load in advance.

## Status

The observer is wired at the **page cache (L2)** so every `fetch_page()` is recorded. It is optional: pass `nullptr` to disable.

## API

### Class `aqa::AccessObserver`

- **Constructor:** `AccessObserver(size_t ring_capacity = 1024)`
  Retains at most `ring_capacity` page access events; oldest are dropped when full.

- **Recording:**
  - `record_page_access(uint32_t page_id, bool is_hit)` — Call on every page fetch (hit or miss). Used by the page cache.
  - `record_record_access(RecordID rid)` — Records a record access (stores `rid.page_id`). Optional; for future use by the storage reader.

- **Read (thread-safe):**
  - `get_recent_page_accesses(size_t n)` — Returns the last `n` events as `std::vector<PageAccessEvent>` (oldest first). Each event has `page_id` and `is_hit`.
  - `get_recent_page_ids(size_t n)` — Returns the last `n` page IDs in order (convenience for prefetchers).
  - `get_access_count(uint32_t page_id)` — Returns how many times `page_id` appears in the current ring (bounded frequency). Used by LFU-style eviction policies.
  - `get_size()` — Current number of events in the ring (≤ capacity).
  - `get_capacity()` — Ring capacity.
  - `get_total_recorded()` — Total number of events ever recorded (may exceed capacity).

### Struct `aqa::PageAccessEvent`

- `uint32_t page_id`
- `bool is_hit`

## Where it is used

| Layer | Usage |
| ----- | ----- |
| **PageCache** | On every `get_page_internal()`: after determining hit/miss, calls `observer_->record_page_access(page_id, is_hit)` when observer is non-null. |
| **StorageEngine** | Accepts optional `AccessObserver*` in the constructor and passes it to `PageCache`. |
| **Database** | Accepts optional `AccessObserver*` as the fourth constructor argument and passes it to `StorageEngine`. Use `get_observer()` to retrieve the pointer. |

Passing `nullptr` (the default) disables observation at that layer; no events are recorded and there is no performance cost.

## Testing

- **Unit tests:** `tests/access_observer_test.cpp` — ring buffer behavior, record/retrieve, wraparound, null observer, and integration with `PageCache` and `Database`.
- Run: `./build/tests/access_observer_test` or `ctest --test-dir build/ -R AccessObserverTest`

## Design notes

- **Thread-safe:** All public methods use an internal mutex so the observer can be used from multiple threads (e.g. concurrent reads).
- **No persistence:** The ring buffer is in-memory only. It is intended for short-term patterns to drive eviction and prefetch; it is not a query log.
- **Optional:** The observer is always an optional pointer; existing code paths that do not pass an observer behave exactly as before.

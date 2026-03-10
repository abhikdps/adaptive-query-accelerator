# Eviction Policy Interface

Both the page cache (L2) and the record cache (L1) can use a pluggable **eviction policy** to decide which entry to evict when the cache is full. The default is built-in LRU when no policy is passed.

## Interfaces

### PageEvictionPolicy (L2)

- **`uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order)`**
  Receives unpinned page IDs in LRU order (oldest first). Must return one of them (the page to evict).

### RecordEvictionPolicy (L1)

- **`size_t choose_victim(const std::vector<const std::vector<uint8_t>*>& keys_lru_order)`**
  Receives pointers to keys in LRU order (oldest first). Must return a valid index; that entry will be evicted.

## Implementations

- **LruPageEvictionPolicy** — Returns `unpinned_page_ids_lru_order.front()` (evict oldest).
- **ScanResistantPageEvictionPolicy** — Uses `AccessObserver::get_recent_page_ids()` to detect sequential runs; prefers evicting pages that are part of a run (page_id±1 in recent history), otherwise falls back to LRU.
- **LfuPageEvictionPolicy** — Uses `AccessObserver::get_access_count()` (frequency in the recent ring); evicts the candidate with smallest count, tie-break by LRU order.
- **ScanResistantThenLfuPageEvictionPolicy** — Combines both: first narrows to pages that look like scan traffic, then among those (or among all if none) evicts by LFU. Good default for mixed point-lookup and scan workloads.
- **HintAwarePageEvictionPolicy** — Reads the thread-local workload hint: when `Scan`, uses scan-resistant behavior; when `PointLookup`, uses LFU. Use with `Database::scan()` (which sets the hint automatically) or `set_workload_hint()` / `ScanScope`.
- **LearnedPageEvictionPolicy** — Linear score (recency, count, scan) with online weight updates. See [learned-eviction.md](learned-eviction.md).
- **LoadedLearnedPageEvictionPolicy** — Load weights from a policy file (from offline trainer); same scoring, no runtime updates. See [offline-policy.md](offline-policy.md).
- **LruRecordEvictionPolicy** — Returns `0` (evict oldest).

## Where it is used

| Component | Usage |
| --------- | ----- |
| **PageCache** | Constructor: `PageCache(..., AccessObserver*, PageEvictionPolicy* eviction_policy = nullptr)`. In `evict()`, builds the list of unpinned page IDs in LRU order and calls `eviction_policy_->choose_victim(...)` when non-null; otherwise evicts the first (LRU). |
| **LruCache** | Constructor: `LruCache(size_t capacity, RecordEvictionPolicy* eviction_policy = nullptr)`. On put when at capacity, builds keys in LRU order and calls `eviction_policy_->choose_victim(...)` when non-null; otherwise evicts index 0. |

StorageEngine and Database do not pass a policy (default nullptr), so behavior is unchanged until you plug in a custom policy. To use an adaptive policy, construct a `ScanResistantThenLfuPageEvictionPolicy` or `HintAwarePageEvictionPolicy` (or others) with the same `AccessObserver*` used by the page cache and pass it to `PageCache`. Use `Database::scan(callback)` for full-table scans so the workload hint is set to `Scan` for the duration; eviction and prefetch then treat the workload appropriately.

## Testing

- **Unit tests:** `tests/eviction_policy_test.cpp` — LRU page/record policies; PageCache and LruCache with explicit policies; ScanResistant, LFU, ScanResistantThenLfu, and HintAware policies; null observer fallback to LRU; single-candidate; PageCache with HintAware policy.
- **Run:** `./build/tests/eviction_policy_test` or `ctest --test-dir build/ -R EvictionPolicyTest`

## Benchmark

- **Eviction benchmark:** `benchmarks/eviction_benchmark.cpp` — Compares LRU vs HintAware under a mixed workload: working set is warmed, then a “scan lead” ages in the cache, then the working set is re-touched (so it is newer than the scan lead), then the rest of the scan runs, then the working set is re-accessed. With **LRU**, the working set is evicted during the scan (oldest first). With **HintAware + Scan hint**, pages that are part of the sequential scan are evicted first, so the working set is preserved and the final re-access gets more hits.
- **Run:** `./build/benchmarks/eviction_benchmark`

### Sample output and interpretation

Example output (parameters: working_set=8, scan_lead=16, scan_tail=80, capacity=32):

```text
Eviction benchmark: working_set=8, scan_lead=16, scan_tail=80, capacity=32
----------------------------------------
[LRU] hits=48, misses=144
[HintAware + Scan hint] hits=80, misses=112
----------------------------------------
```

- **LRU** — Final re-access of the working set (32 fetches) often sees misses because those pages were evicted during the scan. Total hits are dominated by re-use during the initial warm-up and re-touch.
- **HintAware + Scan hint** — During the scan phase, victims are chosen from pages that look like scan traffic (sequential run), so the working set (0..7) is kept. The final 32 working-set fetches then hit the cache, so total hits are higher and misses lower than LRU.

Exact numbers depend on capacity, working set size, and scan length; the benchmark is tuned so that HintAware consistently shows more hits than LRU. If both lines are identical, the workload may need adjustment (e.g. larger observer ring or different scan_lead/scan_tail) so that scan pages are present in the victim list and detected as sequential.

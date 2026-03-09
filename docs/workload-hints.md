# Workload Hints

Workload hints let the application signal whether the current operation is a **point lookup** or a **full scan**. The engine uses this to tune eviction and prefetch: during a scan, it can evict scan pages first (scan-resistant) and prefetch the next pages; during point lookups, it keeps frequently used pages (LFU-style).

## API

### Enum `aqa::WorkloadHint`

- **`PointLookup`** — Default. Random or point access; eviction prefers LFU (or policy default).
- **`Scan`** — Sequential scan in progress; eviction prefers scan-resistant (evict pages that look like scan traffic first).

### Thread-local get/set

- **`void set_workload_hint(WorkloadHint hint)`** — Set the current thread’s workload hint.
- **`WorkloadHint get_workload_hint()`** — Read the current thread’s hint (default: `PointLookup`).

Hints are **thread-local**, so concurrent threads can have different modes (e.g. one thread scanning while another does point lookups).

### RAII scope

- **`ScanScope`** — Constructor sets the hint to `Scan`; destructor restores the previous hint. Use for a bounded scan region without manually clearing the hint.

```cpp
{
    aqa::ScanScope scope;
    // ... do scan ...
}  // hint restored to previous value
```

### Database::scan()

- **`void Database::scan(callback)`** — Runs a full-table scan and sets the workload hint to `Scan` for the duration of the callback. After the scan returns, the hint is reset to `PointLookup`. Use this for all full-table scans so eviction and prefetch behave correctly.

```cpp
db.scan([&](aqa::RecordID rid, const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
    // hint is Scan here
});
// hint is PointLookup here
```

## Where it is used

| Component | Usage |
| --------- | ----- |
| **Database** | `scan(callback)` sets `Scan` before calling the storage reader’s scan and restores `PointLookup` when done. Recovery sets `Scan` during `index_.rebuild()`. |
| **HintAwarePageEvictionPolicy** | Calls `get_workload_hint()` in `choose_victim()`: when `Scan`, uses scan-resistant logic; when `PointLookup`, uses LFU. |
| **Prefetch** | Prefetch runs only inside `StorageReader::scan()`, which is invoked when the app (or recovery) is doing a scan; the hint is set accordingly. |

## Manual hint control

For custom scan loops (e.g. cursor-based iteration), set the hint explicitly:

```cpp
aqa::set_workload_hint(aqa::WorkloadHint::Scan);
// ... scan loop ...
aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
```

Or use `ScanScope` so the hint is always restored on exit (including exceptions).

## Testing

- **Unit tests:** `tests/workload_hint_test.cpp` — default hint, set/get, `ScanScope` restore (nested and previous value), and that `Database::scan()` sets the hint during the callback.
- **Run:** `./build/tests/workload_hint_test` or `ctest --test-dir build/ -R WorkloadHintTest`

## See also

- [Eviction policy](eviction-policy.md) — `HintAwarePageEvictionPolicy` and other policies.
- [Access observer](access-observer.md) — How access history feeds eviction and prefetch.
- [Prefetch](prefetch.md) — Sequential prefetch during scan.

# Offline Policy Export and Load

A **offline training** on an access trace and **loading** the resulting policy at runtime. The engine uses the same linear eviction model as learned eviction but with fixed weights from a file (no online updates).

## Design

- **Train offline:** Run the learned eviction policy in a simulator over a trace of page accesses; the policy’s weights are updated online during the simulation. Save the final weights to a **policy file**.
- **Load at runtime:** `LoadedLearnedPageEvictionPolicy` reads the policy file and uses those weights for scoring. No feedback or weight updates at runtime.

## Policy file format

Plain text:

- Optional comment lines starting with `#`.
- One data line: three space-separated doubles: `w_recency w_count w_scan`.

Example:

```text
# Learned eviction weights (trace accesses=10000)
1.2 0.8 -1.0
```

If the file is missing or unreadable, the policy falls back to default weights `(1, 1, -1)`.

## Offline trainer

- **Executable:** `build/benchmarks/learned_eviction_trainer`
- **Usage:** `learned_eviction_trainer <trace_file> <output_policy_file> [ring_cap=256] [cache_cap=64] [feedback_horizon=64] [lr=0.05]`
- **Trace format:** One page_id per line (decimal). Example:

  ```text
  0
  1
  2
  1
  0
  ...
  ```

- **Behavior:** Simulates a cache of size `cache_cap` with an observer (ring size `ring_cap`) and a `LearnedPageEvictionPolicy` (feedback_horizon, lr). Replays the trace (each line = one page access), runs eviction when the cache is full, then writes the policy’s final weights to `output_policy_file`.

## Runtime: loaded policy

- **Class:** `LoadedLearnedPageEvictionPolicy` in `src/cache/eviction_policy.h`
- **Constructor:** `LoadedLearnedPageEvictionPolicy(AccessObserver* observer, const std::string& policy_file_path)`
- **Eviction:** Same as Learned eviction: score = w_recency*recency + w_count*count + w_scan*scan; evict the page with the smallest score. Weights are fixed (no feedback).

## Usage

1. **Produce a policy (offline):** Capture or generate a trace of page_ids, then run the trainer:

   ```bash
   ./build/benchmarks/learned_eviction_trainer trace.txt policy.txt 256 64 64 0.05
   ```

2. **Use the policy in the engine:** Construct the page cache with a loaded policy:

   ```cpp
   AccessObserver observer(1024);
   LoadedLearnedPageEvictionPolicy policy(&observer, "policy.txt");
   PageCache cache(file, capacity, &observer, &policy);
   ```

## Testing

- **Unit tests:** `tests/eviction_policy_test.cpp` — `test_loaded_learned_eviction_policy` (load a file with `0 0 -1`, check scan page is evicted first), `test_loaded_learned_eviction_policy_missing_file_uses_defaults`.
- **Run:** `./build/tests/eviction_policy_test` or `ctest --test-dir build/ -R EvictionPolicyTest`

## See also

- [learned-eviction.md](learned-eviction.md) — Online learned policy and feature definition.
- [eviction-policy.md](eviction-policy.md) — All eviction policies.

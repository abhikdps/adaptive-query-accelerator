# Lightweight Learned Eviction

A small **in-process model** for page eviction that is trained online on a sliding window of access history. The goal is to predict “will this page be requested again soon?” and evict pages that are unlikely to be reused.

## Design

### Model

- **Type:** Linear score over three features derived from the access observer.
- **Features (per page):**
  1. **Recency** — `1 / (1 + last_index)` where `last_index` is the index of the page’s most recent occurrence in the observer ring (0 = most recent). Not in ring → recency ≈ 0.
  2. **Count** — Access count in the current ring, normalized by capacity: `min(1, count / (1 + capacity))`.
  3. **Scan** — 1 if the page is part of a sequential run (page_id±1 in recent history), 0 otherwise.
- **Score:** `score = w_recency * recency + w_count * count + w_scan * scan`. **Lower score ⇒ evict first.** Initial weights: `(1, 1, -1)` so we favor recent and frequently used pages and penalize scan-like pages.

### Online Learning

- **Feedback:** When we evict page P, we record (P, features of P, `total_recorded` at eviction). After **feedback_horizon** more accesses (e.g. 64), we check: was P requested again? (P appears in the observer’s recent ring ⇒ yes.)
- **Label:** 1 if the evicted page was reused in that window, 0 otherwise.
- **Update:** `w += learning_rate * (2*label - 1) * features`. So if the evicted page was reused (label=1), we increase the score for similar feature vectors (evict such pages less often). If it was not reused (label=0), we decrease (evict similar pages more readily).

### Observer Support

- **`get_last_index_in_recent(page_id)`** — Returns the index of the page’s most recent occurrence in the ring (0 = most recent), or `capacity` if not found. Used for the recency feature.

## Implementation

- **Policy:** `LearnedPageEvictionPolicy` in `src/cache/eviction_policy.h/.cpp`.
- **Constructor:** `LearnedPageEvictionPolicy(AccessObserver* observer, size_t feedback_horizon = 64, double learning_rate = 0.05)`.
- **Eviction:** Same as other policies: receives unpinned page IDs in LRU order, returns the chosen victim (minimum score). Stores the evicted page and its features for the next feedback step.
- **Thread safety:** Uses the observer’s thread-safe API; the policy itself is not thread-safe (use one policy per cache, or external synchronization).

## Usage

Use like any other `PageEvictionPolicy`: construct with the same `AccessObserver*` as the page cache and pass to `PageCache`:

```cpp
AccessObserver observer(1024);
LearnedPageEvictionPolicy policy(&observer, 64, 0.05);
PageCache cache(file, capacity, &observer, &policy);
```

## Testing

- **Unit tests:** `tests/eviction_policy_test.cpp` — `test_learned_eviction_policy` (returns a valid candidate), `test_learned_eviction_policy_null_observer` (fallback to first when observer is null).
- **Run:** `./build/tests/eviction_policy_test` or `ctest --test-dir build/ -R EvictionPolicyTest`

## Limitations and Future Work

- **Single pending feedback:** Only the most recent eviction is used for a weight update; if another eviction happens before `feedback_horizon` accesses, the previous one is discarded.
- **No persistence:** Weights are in-memory only and reset on restart.
- **Fixed features:** Recency, count, and scan are hand-picked; Next we can add learned feature selection or a small neural net.
- **Eviction only:** The same style of model could later drive prefetch (e.g. “prefetch pages with high predicted reuse”).

## See Also

- [eviction-policy.md](eviction-policy.md) — Interface and other policies.
- [access-observer.md](access-observer.md) — Observer API, including `get_last_index_in_recent`.

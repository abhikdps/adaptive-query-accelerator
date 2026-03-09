# B+ Tree Implementation Status

The codebase includes **page-level** B+ tree structures (leaf and internal pages) under `src/storage/page/`. They compile and are linked into the core library but are **not yet integrated** with the storage engine or the Database.

## What Exists

| Component | Description |
|-----------|-------------|
| **BPlusTreePage** | Base class: page type, size, max/min size, parent/page id, LSN. Uses `page_id_t` (= `int`) and `lsn_t` (= `int`). |
| **BPlusTreeLeafPage&lt;K,V,C&gt;** | Leaf: `Init`, `KeyAt`, `ValueAt`, `KeyIndex`, `Insert`, `GetNextPageId` / `SetNextPageId`. Template instantiation: `int, int, std::less<int>`. |
| **BPlusTreeInternalPage&lt;K,V,C&gt;** | Internal: `Init`, `KeyAt`, `ValueAt`, `Lookup`, `ValueIndex`, get/set key/value. Template instantiation: `int, int, std::less<int>`. |

- **Leaf:** Insert rejects duplicates (returns -1), rejects when full (returns -1), otherwise inserts in sorted order and returns the new size.
- **Internal:** Lookup uses `upper_bound` with the usual “first key unused” convention; returns default `ValueType{}` when size is 0 to avoid undefined behavior.

## Fixes Applied

1. **Leaf `Insert`** — Now checks `current_size >= getMaxSize()` and returns -1 when full (no overflow).
2. **Indentation** — `getPageId()` in `b_plus_tree_page.cpp` fixed.
3. **Internal `Lookup`** — When `getSize() == 0`, returns `ValueType{}` instead of reading uninitialized memory.

## What Is Not Done

- **No storage integration** — Page types are in-memory views. They are not backed by `RawPage` / `MappedFile` / `StorageEngine`. There is no `BPlusTree` or `BPlusTreeManager` that allocates or fetches pages from the engine.
- **No tree operations** — No insert/lookup/split/merge at the tree level; only single-page leaf insert and internal lookup.
- **Type mismatch** — B+ tree uses `page_id_t` = `int`; the rest of AQA uses `uint32_t` for page IDs. Will need alignment when wiring to the engine.
- **Layout vs RawPage** — Current storage uses `RawPage` (64-byte header + slotted payload). B+ tree pages use their own header sizes (`LEAF_PAGE_HEADER_SIZE` 28, `INTERNAL_PAGE_HEADER_SIZE` 24) and a key/value array. To persist B+ tree pages, either define a separate B+ tree file format or reserve a page type in the existing format and map it.

## Summary

The B+ tree **page** implementation is in place and corrected for the issues above. To use it as an index we can add a **B+ tree layer** that: uses `StorageEngine` (or a dedicated file) to fetch/allocate 4KB pages, casts buffers to these page types, and implements tree insert/lookup/split/merge. The current Database continues to use the hash index only.

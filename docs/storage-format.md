# Storage Format Specifications

## Overview

The AQA (Adaptive Query Accelerator) uses a fixed size, page based storage engine. This design minimizes the disk I/O overhead and optimizes for CPU cache usage. The default page size is **4KB** (4096 bytes), matching the default virtual memory page size of most modern OS kernel and SSD hardware block sizes.

## Page Layout

Every Page is divided into two distinct regions: the **Header** and the **Payload**.

```text
+------------------------------------------------------+
|  Page Layout (4096 bytes)                            |
+------------------------------------------------------+
|  HEADER (16 bytes)                                   |
|  - Magic Number (4B)                                 |
|  - Page ID (4B)                                      |
|  - LSN (Log Sequence Num) (4B)                       |
|  - Next Page ID (4B)                                 |
+------------------------------------------------------+ <--- Offset 16
|                                                      |
|  PAYLOAD / RAW DATA STORAGE                          |
|  (4080 bytes)                                        |
|                                                      |
|  [Used by Nodes, Tuples, or Blobs]                   |
|                                                      |
|                                                      |
+------------------------------------------------------+ <--- Offset 4096
```

## Field Descriptions

| Offset | Size (Bytes) | Field          | Description                                                         |
|--------|--------------|----------------|---------------------------------------------------------------------|
| 0      | 4            | `magic`        | Unique signature `(0xC0DEFACE)` to verify page validity.            |
| 4      | 4            | `page_id`      | The unique index of this page in the file (0-based).                |
| 8      | 4            | `lsn`          | Log Sequence Number. Reserved for Write-Ahead Logging (WAL).        |
| 12     | 4            | `next_page_id` | Pointer to the next page. Used for linked lists (overflow pages).   |
| 16     | 4080         | `payload`      | Raw data area (Nodes, Tuples, etc.)                                 |

## Design Decisions

1. **Mmap Alignment:** The file is a simple concatenation of these 4KB pages. This allows `mmap` to map the file 1:1 into memory without any padding or complex deserialization.

2. **Compact Header:** We reduced the header to the bare minimum (16 bytes) to maximize payload space (4080 bytes). Alignment padding is not strictly necessary here as 16 bytes is already aligned to standard word boundaries.

3. **Future Extensibility:** While the current implementation exposes a raw payload, higher-level abstractions (like B+ Tree Nodes) will cast this payload region into their own specific structures (e.g., `LeafNode` or `InternalNode`).

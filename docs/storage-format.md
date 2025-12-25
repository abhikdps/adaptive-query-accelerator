# Storage Format Specifications

## Overview

The ADA (Adaptive Query Accelerator) uses a fixed size, page based storage engine. This design minimizes the disk I/O overhead and optimizes for CPU cache usage. The default page size is **4KB** (4096 bytes), matching the default virtual memory page size of most modern OS kernel and SSD hardware block sizes.

## Page Layout

Every Page is divided into two distint regions: the **Header** and the **Payload**.

```text
+------------------------------------------------------+
|  Page Layout (4096 bytes)                            |
+------------------------------------------------------+
|  HEADER (64 bytes)                                   |
|  - Magic Number (4B)                                 |
|  - Page ID (4B)                                      |
|  - Links (Next/Prev) (8B)                            |
|  - Metadata (Offsets/Counts) (8B)                    |
|  - Padding (36B) -> Aligns to Cache Line             |
+------------------------------------------------------+ <--- Offset 64
|                                                      |
|  PAYLOAD / RAW DATA STORAGE                          |
|  (4032 bytes)                                        |
|                                                      |
|  [Tuple Data grows Downwards v]                      |
|                                                      |
|             (Free Space)                             |
|                                                      |
|  [Slot Array grows Upwards ^]                        |
|                                                      |
+------------------------------------------------------+ <--- Offset 4096
```

## Design Decisions

1. Cache Line Alignment
The header is padded to 64 bytes. Even though the actual metadata only requires 28 bytes, keeping the header aligned ensures that accessing the first tuple (at offset 64) or reading the header does not straddle cache lines.

2. Variable Length Avoidance
The PageHeader struct contains no pointers or variable-length arrays. All referencing is done via integer offsets relative to the start of the payload buffer.

3. Slotted Page Architecture Support
While the current implementation defines a raw payload, the `free_space_start` and `free_space_end` fields are included to support a Slotted Page architecture in the future.

    - Slot Array: Will grow from the "top" of the payload (low address).

    - Tuple Data: Will be inserted at the "bottom" of the payload (high address).

    - This allows the page to store variable-length records while keeping the header fixed.

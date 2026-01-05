# Storage Format Specifications v2

**Architecture:** Slotted Page with variable-length records.

## 1. File Structure

The database file (`.db`) is a linear sequence of 4KB fixed-size pages.

```text
[Page 0] [Page 1] [Page 2] ... [Page N]
```

## 2. Page Layout (4KB)

Each page organizes data using a **Slotted Page** approach. The page grows from both ends towards the middle.

- **Header & Slots:** Grow **Upwards** (Low Address -> High Address)
- **Record Data:** Grows **Downwards** (High Address -> Low Address)
- **Free Space:** The gap in the middle. The page is "Full" when these regions meet.

```text
+-------------------------------------------------+ 0
|   Page Header (16 Bytes)                        |
+-------------------------------------------------+ 16
|   Slot Count (2 Bytes)                          |
+-------------------------------------------------+ 18
|   Slot[0] {Offset, Len} (B bytes)               |
|   Slot[0] {Offset, Len} (B bytes)               |
|   ...                                           |
|   Slot[k]                                       |
+-------------------------------------------------+
|                                                 |
|                 FREE SPACE GAP                  |
|       (variable Size, decreases on write)       |
|                                                 |
+-------------------------------------------------+
|   Record[k] Data                                |
+-------------------------------------------------+
|   ...                                           |
+-------------------------------------------------+
|   Record[1] Data                                |
+-------------------------------------------------+
|   Record[0] Data                                |
+-------------------------------------------------+ 4096
```

## 3. Data Structures

### A. Record ID (RID)

A gloablly unique identifier for any record in the database.

- **Size:** 64 bits (8 bytes)
- **Format:** `[ Page ID (32 bits) ] [ Slot Index (16 bits) ] [ Reserved (16 bits) ]`
- **Usage:** To find a record, we fetch `Page(PageID)`, then look up `Slot[SlotIndex]`.

### B. Slot Entry

Each entry in the Slot Array is a 4-byte descriptor.

- **Offset (16 bits):** Byte offset of the record start relative to the Page start.
- **Length (16 bits):** Total length of the record (Header + Key + Value).

### C. Record Format

The actual data stored in the payload heap. It supports variable-length keys and values.

| Field | Size | Type | Description |
| :--- | :--- | :--- | :--- |
| **Meta** | 1 Byte | `uint8_t` | Flags (see below). |
| **Key Len** | 2 Bytes | `uint16_t` | Length of the Key in bytes. |
| **Val Len** | 2 Bytes | `uint16_t` | Length of the Value in bytes. |
| **Key** | *Var* | `char[]` | The key data. |
| **Value** | *Var* | `char[]` | The value data. |

**Metadata Flags:**

- `0x01` (**TOMBSTONE**): The record is marked as deleted.
- `0x00` (Alive): Valid record.

## 4. Visual Diagram

```mermaid
classDiagram
    class Page {
        +Header (16B)
        +SlotCount (2B)
        +SlotArray[]
        +FreeSpace
        +Records[]
    }

    class Slot {
        +Offset: uint16
        +Length: uint16
    }

    class Record {
        +Flags: uint8
        +KeyLen: uint16
        +ValLen: uint16
        +KeyBytes
        +ValueBytes
    }

    Page *-- Slot : Contains 0..N
    Page *-- Record : Contains 0..N
    Slot ..> Record : Points to
```

## 5. Write Mechanics (Append-Only)

When inserting a new record:

1. **Check Capacity:** Ensure `FreeSpace >= sizeof(Slot) + sizeof(Record)`.
2. **Append Data:** Copy the `Record` bytes to the **end** of the Free Space (growing downwards).
3. **Add Slot:** Append a new `Slot` entry to the **start** of the Free Space (growing upwards).
4. **Update Count:** Increment `Slot Count`.

***Note:** For B+ Trees, the Slot Array is typically lept sorted by the Key to allow Binary Search within the page, even in the Record Data is unsorted in the heap.*

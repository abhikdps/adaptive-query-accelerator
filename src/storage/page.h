#ifndef ADAPTIVE_QUERY_ACCELERATOR_PAGE_H
#define ADAPYIVE_QUERY_ACCELERATOR_PAGE_H

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace aqa {
    // Page size
    constexpr uint32_t PAGE_SIZE = 4096; // Default to 4KB

    // Magic number to verify page integrity
    constexpr uint32_t PAGE_MAGIC = 0x41514144; // ASCII FOR AQAD

    struct alignas(64) PageHeader {
        uint32_t magic;
        uint32_t page_id;
        uint32_t next_page_id;
        uint32_t prev_page_id;
        uint16_t tuple_count;
        uint16_t free_space_start;
        uint16_t free_space_end;
        uint16_t flags;
        uint8_t padding[36];
    };

    static_assert(sizeof(PageHeader) == 64, "PageHeader must be 64 bytes");

    struct Page {
        PageHeader header;

        uint8_t payload[PAGE_SIZE - sizeof(PageHeader)];

        Page() {
            reset();
        }

        void reset() {
            std::memset(this, 0, PAGE_SIZE);
            header.magic = PAGE_MAGIC;
            header.next_page_id = 0xFFFFFFFF; // invalid id
            header.prev_page_id = 0xFFFFFFFF; // invalid id
            header.free_space_start = 0;
            header.free_space_end = sizeof(payload);
        }
    };

    static_assert(sizeof(Page) == PAGE_SIZE, "Page struct must match configured PAGE_SIZE");
}

#endif

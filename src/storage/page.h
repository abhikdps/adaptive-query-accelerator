#ifndef ADAPTIVE_QUERY_ACCELERATOR_PAGE_H
#define ADAPTIVE_QUERY_ACCELERATOR_PAGE_H

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>

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

    struct alignas(PAGE_SIZE) RawPage {
        PageHeader header;
        uint8_t payload[PAGE_SIZE- sizeof(PageHeader)];

        RawPage() {
            reset();
        }

        void reset() {
            std::memset(this, 0,PAGE_SIZE);
            header.magic = PAGE_MAGIC;
        }
    };

    static_assert(sizeof(RawPage) == PAGE_SIZE, "RawPage must be 4KB");

    class Page {
        public:
            static constexpr size_t PAYLOAD_SIZE = sizeof(RawPage::payload);

            explicit Page(RawPage* ptr) : ptr_(ptr) {
                if (!ptr_) {
                    throw std::runtime_error("Page initialized with nullptr");
                }
            }

            [[nodiscard]] uint32_t get_id() const {
                return ptr_->header.page_id;
            }

            [[nodiscard]] const PageHeader& get_header() const {
                return ptr_->header;
            }

            [[nodiscard]] std::span<const uint8_t> get_payload() const {
                return {ptr_->payload, PAYLOAD_SIZE};
            }

            [[nodiscard]] bool is_valid() const {
                return ptr_->header.magic == PAGE_MAGIC;
            }

            PageHeader& get_header_mut() {
                return ptr_->header;
            }

            std::span<uint8_t> get_payload_mut() {
                return {ptr_->payload, PAYLOAD_SIZE};
            }

            void reset() {
                std::memset(ptr_, 0, PAGE_SIZE);
                ptr_->header.magic = PAGE_MAGIC;
            }

        private:
            RawPage* ptr_;
    };
}

#endif

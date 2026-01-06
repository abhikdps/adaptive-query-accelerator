#include "storage/storage_reader.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <iostream>
#include <vector>
#include "storage/page.h"
#include "storage/storage_engine.h"

namespace aqa {
    constexpr size_t PAGE_HEADER_SIZE = 64;
    constexpr size_t SLOT_SIZE = 4;

    struct SlotEntry {
        uint16_t offset;
        uint16_t length;
    };

    struct RecordHeader {
        uint8_t meta;
        uint16_t key_len;
        uint16_t val_len;
    } __attribute__((packed));

    StorageReader::StorageReader(StorageEngine& engine) : engine_(engine) {}

    std::optional<Record> StorageReader::read_record(RecordID rid) {
        if (rid.page_id >= engine_.get_total_pages()) {
            return std::nullopt;
        }

        auto page_handle = engine_.fetch_page(rid.page_id);
        const uint8_t* raw_data = reinterpret_cast<const uint8_t*>(page_handle->get_raw_data());

        const uint16_t* slot_count_ptr = reinterpret_cast<const uint16_t*>(raw_data + PAGE_HEADER_SIZE);
        uint16_t slot_count = *slot_count_ptr;

        if (rid.slot_id >= slot_count) {
            return std::nullopt;
        }

        size_t slots_start = PAGE_HEADER_SIZE + sizeof(uint16_t);
        size_t slot_offset = slots_start + (rid.slot_id * SLOT_SIZE);

        const SlotEntry* slot = reinterpret_cast<const SlotEntry*>(raw_data + slot_offset);

        if (slot->offset >= PAGE_SIZE || (slot->offset + slot->length) > PAGE_SIZE) {
            std::cerr << "Corruption: Record out of bounds!" << std::endl;
            return std::nullopt;
        }

        const uint8_t* record_ptr = raw_data + slot->offset;
        const RecordHeader* header = reinterpret_cast<const RecordHeader*>(record_ptr);

        if (header->meta == 0x01) {
            return std::nullopt;
        }

        Record result;
        const uint8_t* key_ptr = record_ptr + sizeof(RecordHeader);
        const uint8_t* val_ptr = key_ptr + header->key_len;

        result.key.assign(key_ptr, key_ptr + header->key_len);
        result.value.assign(val_ptr, val_ptr + header->val_len);

        return result;
    }

    void StorageReader::scan(std::function<void(RecordID, const std::vector<uint8_t>&, const std::vector<uint8_t>&)> callback) {
        uint32_t total_pages = engine_.get_total_pages();

        for (uint32_t pid = 0; pid < total_pages; ++pid) {
            auto page_handle = engine_.fetch_page(pid);
            const uint8_t* raw_data = reinterpret_cast<const uint8_t*>(page_handle->get_raw_data());

            const uint16_t* slot_count_ptr = reinterpret_cast<const uint16_t*>(raw_data + PAGE_HEADER_SIZE);
            uint16_t slot_count = *slot_count_ptr;

            for (uint16_t sid = 0; sid < slot_count; ++sid) {
                RecordID rid = {pid, sid};

                auto record_opt = read_record(rid);

                if (record_opt.has_value()) {
                    callback(rid, record_opt->key, record_opt->value);
                }
            }
        }
    }
}

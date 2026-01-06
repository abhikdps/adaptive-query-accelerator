#include "storage/storage_writer.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "storage/page.h"

namespace aqa {
    constexpr size_t PAGE_HEADER_SIZE = 16;
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

    StorageWriter::StorageWriter(const std::string& path)
        : path_(path), current_page_id_(0) {
        out_stream_.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!out_stream_.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + path);
        }

        reset_current_page();
    }

    StorageWriter::~StorageWriter() {
        flush();
        if (out_stream_.is_open()) {
            out_stream_.close();
        }
    }

    void StorageWriter::reset_current_page() {
        std::memset(reinterpret_cast<void*>(&current_page_), 0, PAGE_SIZE);

        current_page_.header.magic = PAGE_MAGIC;
        current_page_.header.page_id = current_page_id_;
        current_page_.header.next_page_id = 0xFFFFFFFF;

        current_slot_count_ = 0;

        free_space_offset_ = PAGE_SIZE;
    }

    void StorageWriter::write_page_to_disk() {
        uint16_t* slot_count_ptr = reinterpret_cast<uint16_t*>(current_page_.payload);
        *slot_count_ptr = current_slot_count_;

        out_stream_.write(reinterpret_cast<const char*>(&current_page_), PAGE_SIZE);
        out_stream_.flush();
    }

    void StorageWriter::flush() {
        if (current_slot_count_ > 0 || current_page_id_ == 0) {
            write_page_to_disk();
        }
    }

    RecordID StorageWriter::append(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
        uint16_t slot_id = 0;

        if (!try_insert_in_page(key, value, slot_id)) {
            write_page_to_disk();
            current_page_id_++;
            reset_current_page();

            if (!try_insert_in_page(key, value, slot_id)) {
                throw std::runtime_error("Record too large for a single page!");
            }
        }

        return {current_page_id_, slot_id};
    }

    bool StorageWriter::try_insert_in_page(const std::vector<uint8_t>& key,
                                            const std::vector<uint8_t>& value,
                                            uint16_t& out_slot_id) {
        size_t record_data_size = sizeof(RecordHeader) + key.size() + value.size();
        size_t new_slot_size = SLOT_SIZE;

        size_t slots_start_offset = sizeof(uint16_t);
        size_t slots_end_offset = slots_start_offset + (current_slot_count_ * SLOT_SIZE);

        size_t required_space = new_slot_size + record_data_size;
        size_t free_space = free_space_offset_ - (PAGE_HEADER_SIZE + slots_end_offset);

        if (free_space < required_space) {
            return false;
        }

        uint16_t new_record_offset = free_space_offset_ - record_data_size;
        uint8_t* record_ptr = reinterpret_cast<uint8_t*>(&current_page_) + new_record_offset;

        RecordHeader header;
        header.meta = 0x00;
        header.key_len = static_cast<uint16_t>(key.size());
        header.val_len = static_cast<uint16_t>(value.size());

        std::memcpy(record_ptr, &header, sizeof(RecordHeader));
        std::memcpy(record_ptr + sizeof(RecordHeader), key.data(), key.size());
        std::memcpy(record_ptr + sizeof(RecordHeader) + key.size(), value.data(), value.size());

        uint8_t* payload_base = current_page_.payload;
        uint8_t* slot_ptr = payload_base + slots_end_offset;

        SlotEntry slot;
        slot.offset = new_record_offset;
        slot.length = static_cast<uint16_t>(record_data_size);

        std::memcpy(slot_ptr, &slot, sizeof(SlotEntry));

        out_slot_id = current_slot_count_;
        current_slot_count_++;
        free_space_offset_ = new_record_offset;

        return true;
    }
}

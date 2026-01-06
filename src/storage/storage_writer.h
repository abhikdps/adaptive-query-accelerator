#ifndef ADAPTIVE_QUERY_ACCELERATOR_STORAGE_WRITER_H
#define ADAPTIVE_QUERY_ACCELERATOR_STORAGE_WRITER_H

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include "storage/page.h"

namespace aqa {
    struct RecordID {
        uint32_t page_id;
        uint16_t slot_id;
    };

    class StorageWriter {
        public:
            explicit StorageWriter(const std::string& path);
            ~StorageWriter();

            RecordID append(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value);

            void flush();

        private:
            void reset_current_page();
            void write_page_to_disk();
            bool try_insert_in_page(const std::vector<uint8_t>& key,
                                    const std::vector<uint8_t>& value,
                                    uint16_t& out_slot_id);

            std::string path_;
            std::ofstream out_stream_;

            RawPage current_page_;
            uint32_t current_page_id_;
            uint16_t current_slot_count_;
            uint16_t free_space_offset_;
    };
}

#endif

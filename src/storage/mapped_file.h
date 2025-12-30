#ifndef ADAPTIVE_QUERY_ACCELERATOR_MAPPED_FILE_H
#define ADAPTIVE_QUERY_ACCELERATOR_MAPPED_FILE_H

#include <string>
#include <cstdint>
#include <cstddef>
#include "storage/page.h"

namespace aqa {
    class MappedFile {
        public:
            explicit MappedFile(const std::string& path);
            ~MappedFile();

            MappedFile(const MappedFile&) = delete;
            MappedFile& operator=(const MappedFile&) = delete;

            Page* get_page(uint32_t page_id);

            void grow_file(uint32_t new_page_count);

            void flush();

            [[nodiscard]] uint32_t get_page_count() const { return page_count_; }
            [[nodiscard]] size_t get_size() const { return file_size_; }

        private:
            std::string path_;
            int fd_;
            void* data_;
            size_t file_size_;
            uint32_t page_count_;

            void map_memory();
            void unmap_memory();
    };
}

#endif

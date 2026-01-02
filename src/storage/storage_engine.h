#ifndef ADAPTIVE_QUERY_ACCELERATOR_STORAGE_ENGINE_H
#define ADAPTIVE_QUERY_ACCELERATOR_STORAGE_ENGINE_H

#include <string>
#include <memory>
#include "storage/page_cache.h"
#include "storage/page_handle.h"
#include "storage/mapped_file.h"

namespace aqa {
    class StorageEngine {
        public:
            StorageEngine(const std::string& file_path, size_t cache_capacity = 1000);

            StorageEngine(const StorageEngine&) = delete;
            StorageEngine& operator=(const StorageEngine&) = delete;

            PageHandle fetch_page(uint32_t page_id);

            PageHandle allocate_page();

            void flush_all();

            uint32_t get_total_pages() const;

            size_t get_cache_hits() const { return cache_->get_hits(); }
            size_t get_cache_misses() const { return cache_->get_misses(); }

        private:
            std::unique_ptr<MappedFile> file_;
            std::unique_ptr<PageCache> cache_;
    };
}

#endif

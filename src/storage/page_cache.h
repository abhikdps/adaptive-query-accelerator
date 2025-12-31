#ifndef ADAPTIVE_QUERY_ACCELERATOR_PAGE_CACHE_H
#define ADAPTIVE_QUERY_ACCELERATOR_PAGE_CACHE_H

#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <optional>
#include "storage/page.h"
#include "storage/mapped_file.h"

namespace aqa {
    class PageCache {
        public:
            PageCache(MappedFile& file, size_t capacity);

            Page get_page(uint32_t page_id);

            void flush_page(uint32_t page_id);

            void flush_all();

            size_t get_hits() const { return hits_; }
            size_t get_misses() const { return misses_; }
            size_t get_capacity() const { return capacity_; }
            size_t get_size() const { return page_map_.size(); }

        private:
            MappedFile& file_;
            size_t capacity_;

            std::vector<RawPage> pool_;
            std::unordered_map<uint32_t, size_t> page_map_;

            std::list<uint32_t> lru_list_;
            std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_iters_;

            std::list<size_t> free_frames_;

            mutable std::mutex mutex_;

            size_t hits_ = 0;
            size_t misses_ = 0;

            void touch_page(uint32_t page_id);

            size_t evict();
    };
}

#endif

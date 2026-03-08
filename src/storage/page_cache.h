#ifndef ADAPTIVE_QUERY_ACCELERATOR_PAGE_CACHE_H
#define ADAPTIVE_QUERY_ACCELERATOR_PAGE_CACHE_H

#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>

#include "storage/page.h"
#include "storage/mapped_file.h"
#include "storage/page_handle.h"

namespace aqa {
    class AccessObserver;

    class PageCache {
        public:
            PageCache(MappedFile& file, size_t capacity, AccessObserver* observer = nullptr);

            // DELETE COPY: Cache is a singleton resource
            PageCache(const PageCache&) = delete;
            PageCache& operator=(const PageCache&) = delete;

            // DELETE MOVE: Moving a cache while threads
            PageCache(PageCache&&) = delete;
            PageCache& operator=(PageCache&&) = delete;

            PageHandle fetch_page(uint32_t page_id);

            void flush_page(uint32_t page_id);

            void flush_all();

            size_t get_hits() const { return hits_; }
            size_t get_misses() const { return misses_; }
            size_t get_capacity() const { return pool_.size(); }
            size_t get_size() const { return page_map_.size(); }

            int get_pin_count_for_test(uint32_t page_id);
        private:
            friend class PageHandle;

            Page get_page_internal(uint32_t page_id);

            void unpin_page(uint32_t page_id);

            MappedFile& file_;

            std::vector<RawPage> pool_;
            std::vector<int> pin_counts_;
            std::unordered_map<uint32_t, size_t> page_map_;

            std::list<uint32_t> lru_list_;
            std::unordered_map<uint32_t, std::list<uint32_t>::iterator> lru_iters_;

            std::list<size_t> free_frames_;

            mutable std::mutex mutex_;

            size_t hits_ = 0;
            size_t misses_ = 0;

            AccessObserver* observer_;

            void touch_page(uint32_t page_id);

            size_t evict();
    };
}

#endif

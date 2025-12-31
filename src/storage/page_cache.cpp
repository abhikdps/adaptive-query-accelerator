#include "storage/page_cache.h"
#include <iostream>
#include <cstring>

namespace aqa {
    PageCache::PageCache(MappedFile& file, size_t capacity)
        : file_(file), capacity_(capacity) {
            pool_.resize(capacity);
            for (size_t i = 0; i < capacity; ++i) {
                free_frames_.push_back(i);
            }
        }

    Page PageCache::get_page(uint32_t page_id) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (page_map_.find(page_id) != page_map_.end()) {
            hits_++;
            touch_page(page_id);

            size_t frame_id = page_map_[page_id];
            return Page(&pool_[frame_id]);
        }

        misses_++;
        size_t frame_id;

        if (!free_frames_.empty()) {
            frame_id = free_frames_.front();
            free_frames_.pop_front();
        } else {
            frame_id = evict();
        }

        RawPage* src = file_.get_page(page_id);
        std::memcpy(&pool_[frame_id], src, PAGE_SIZE);

        page_map_[page_id] = frame_id;
        lru_list_.push_front(page_id);
        lru_iters_[page_id] = lru_list_.begin();

        return Page(&pool_[frame_id]);
    }

    void PageCache::touch_page(uint32_t page_id) {
        auto it = lru_iters_[page_id];
        lru_list_.erase(it);
        lru_list_.push_front(page_id);
        lru_iters_[page_id] = lru_list_.begin();
    }

    size_t PageCache::evict() {
        uint32_t victim_page_id = lru_list_.back();
        size_t frame_id = page_map_[victim_page_id];

        lru_list_.pop_back();
        lru_iters_.erase(victim_page_id);
        page_map_.erase(victim_page_id);

        return frame_id;
    }

    void PageCache::flush_page(uint32_t page_id) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (page_map_.count(page_id)) {
            size_t frame_id = page_map_[page_id];
            RawPage* cached_data = &pool_[frame_id];
            RawPage* disk_data = file_.get_page(page_id);

            std::memcpy(disk_data, cached_data, PAGE_SIZE);
        }
    }

    void PageCache::flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto const& [pid, fid] :  page_map_) {
            RawPage* disk_data = file_.get_page(pid);
            std::memcpy(disk_data, &pool_[fid], PAGE_SIZE);
        }

        file_.flush();
    }
}

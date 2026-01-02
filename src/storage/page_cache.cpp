#include "storage/page_cache.h"
#include <cstring>
#include <stdexcept>

namespace aqa {
    PageHandle::PageHandle(PageCache* cache, Page page, uint32_t page_id)
        : cache_(cache), page_(page), page_id_(page_id) {}

    PageHandle::~PageHandle() {
        if (cache_) {
            cache_->unpin_page(page_id_);
        }
    }

    PageHandle::PageHandle(PageHandle&& other) noexcept
        : cache_(other.cache_), page_(other.page_), page_id_(other.page_id_) {
        other.cache_ = nullptr;
    }

    PageHandle& PageHandle::operator=(PageHandle&& other) noexcept {
        if (this != &other) {
            if (cache_) {
                cache_->unpin_page(page_id_);
            }
            cache_ = other.cache_;
            page_ = other.page_;
            page_id_ = other.page_id_;
            other.cache_ = nullptr;
        }
        return *this;
    }

    PageCache::PageCache(MappedFile& file, size_t capacity)
        : file_(file) {

        pool_.resize(capacity);
        pin_counts_.resize(capacity, 0);

        for (size_t i = 0; i < capacity; ++i) {
            free_frames_.push_back(i);
        }
    }

    PageHandle PageCache::fetch_page(uint32_t page_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        Page p = get_page_internal(page_id);
        return PageHandle(this, p, page_id);
    }

    Page PageCache::get_page_internal(uint32_t page_id) {
        if (page_map_.find(page_id) != page_map_.end()) {
            hits_++;
            touch_page(page_id);

            size_t frame = page_map_[page_id];
            pin_counts_[frame]++;
            return Page(&pool_[frame]);
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

        pin_counts_[frame_id] = 1;

        return Page(&pool_[frame_id]);
    }

    void PageCache::unpin_page(uint32_t page_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (page_map_.find(page_id) != page_map_.end()) {
            size_t frame = page_map_[page_id];
            if (pin_counts_[frame] > 0) {
                pin_counts_[frame]--;
            }
        }
    }

    void PageCache::touch_page(uint32_t page_id) {
        auto it = lru_iters_[page_id];
        lru_list_.erase(it);
        lru_list_.push_front(page_id);
        lru_iters_[page_id] = lru_list_.begin();
    }

    size_t PageCache::evict() {
        for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
            uint32_t victim_id = *it;
            size_t frame = page_map_[victim_id];
            if (pin_counts_[frame] == 0) {
                RawPage* disk_data = file_.get_page(victim_id);
                if (disk_data) {
                    std::memcpy(disk_data, &pool_[frame], PAGE_SIZE);
                }
                page_map_.erase(victim_id);
                lru_iters_.erase(victim_id);

                auto forward_it = std::next(it).base();
                lru_list_.erase(forward_it);

                return frame;
            }
        }

        throw std::runtime_error("Buffer Pool Full: All pages are pinned..");
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
        for (auto const& [pid, fid] : page_map_) {
            RawPage* disk_data = file_.get_page(pid);
            if (disk_data) {
                std::memcpy(disk_data, &pool_[fid], PAGE_SIZE);
            }
        }

        file_.flush();
    }

    int PageCache::get_pin_count_for_test(uint32_t page_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (page_map_.count(page_id)) {
            return pin_counts_[page_map_[page_id]];
        }
        return 0;
    }
}

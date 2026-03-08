#include "storage/storage_engine.h"
#include "observer/access_observer.h"
#include <stdexcept>
#include <thread>

namespace aqa {
    StorageEngine::StorageEngine(const std::string& file_path, size_t cache_capacity,
                                 AccessObserver* observer) {
        file_ = std::make_unique<MappedFile>(file_path);
        cache_ = std::make_unique<PageCache>(*file_, cache_capacity, observer);
    }

    PageHandle StorageEngine::fetch_page(uint32_t page_id) {
        if (page_id >= file_->get_page_count()) {
            throw std::runtime_error("Page ID out of bounds: " + std::to_string(page_id));
        }

        return cache_->fetch_page(page_id);
    }

    void StorageEngine::prefetch_page(uint32_t page_id) {
        if (page_id >= file_->get_page_count()) {
            return;
        }
        std::thread([this, page_id]() {
            try {
                PageHandle h = fetch_page(page_id);
                (void)h;
            } catch (...) {
            }
        }).detach();
    }

    PageHandle StorageEngine::allocate_page() {
        uint32_t new_page_id = file_->get_page_count();
        file_->grow_file(new_page_id + 1);

        PageHandle handle = cache_->fetch_page(new_page_id);

        auto& header = handle->get_header_mut();

        header.page_id = new_page_id;
        header.magic = PAGE_MAGIC;
        header.next_page_id = 0xFFFFFFFF;

        return handle;
    }

    void StorageEngine::flush_all() {
        cache_->flush_all();
    }

    uint32_t StorageEngine::get_total_pages() const {
        return file_->get_page_count();
    }
}

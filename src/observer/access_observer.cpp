#include "observer/access_observer.h"
#include <algorithm>
#include <cstddef>

namespace aqa {

    AccessObserver::AccessObserver(size_t ring_capacity)
        : capacity_(ring_capacity > 0 ? ring_capacity : 1),
        ring_(capacity_) {}

    void AccessObserver::record_page_access(uint32_t page_id, bool is_hit) {
        std::lock_guard<std::mutex> lock(mutex_);
        ring_[head_] = PageAccessEvent{page_id, is_hit};
        head_ = (head_ + 1) % capacity_;
        if (size_ < capacity_) {
            ++size_;
        }
        ++total_recorded_;
    }

    void AccessObserver::record_record_access(RecordID rid) {
        record_page_access(rid.page_id, false);
    }

    std::vector<PageAccessEvent> AccessObserver::get_recent_page_accesses(size_t n) const {
        std::lock_guard<std::mutex> lock(mutex_);
        n = (std::min)(n, size_);
        if (n == 0) {
            return {};
        }
        std::vector<PageAccessEvent> out;
        out.reserve(n);
        size_t idx = (head_ + capacity_ - size_) % capacity_;
        for (size_t i = 0; i < n; ++i) {
            out.push_back(ring_[idx]);
            idx = (idx + 1) % capacity_;
        }
        return out;
    }

    std::vector<uint32_t> AccessObserver::get_recent_page_ids(size_t n) const {
        auto events = get_recent_page_accesses(n);
        std::vector<uint32_t> ids;
        ids.reserve(events.size());
        for (const auto& e : events) {
            ids.push_back(e.page_id);
        }
        return ids;
    }

    size_t AccessObserver::get_size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

}

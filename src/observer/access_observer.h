#ifndef ADAPTIVE_QUERY_ACCELERATOR_ACCESS_OBSERVER_H
#define ADAPTIVE_QUERY_ACCELERATOR_ACCESS_OBSERVER_H

#include "storage/page.h"
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace aqa {

    struct PageAccessEvent {
        uint32_t page_id{0};
        bool is_hit{false};
    };

    class AccessObserver {
        public:
            explicit AccessObserver(size_t ring_capacity = 1024);

            AccessObserver(const AccessObserver&) = delete;
            AccessObserver& operator=(const AccessObserver&) = delete;

            void record_page_access(uint32_t page_id, bool is_hit);

            void record_record_access(RecordID rid);

            std::vector<PageAccessEvent> get_recent_page_accesses(size_t n) const;

            std::vector<uint32_t> get_recent_page_ids(size_t n) const;

            uint64_t get_access_count(uint32_t page_id) const;

            size_t get_last_index_in_recent(uint32_t page_id) const;

            size_t get_total_recorded() const { return total_recorded_; }

            size_t get_size() const;

            size_t get_capacity() const { return capacity_; }

        private:
            size_t capacity_;
            std::vector<PageAccessEvent> ring_;
            std::unordered_map<uint32_t, uint64_t> page_access_counts_;
            size_t head_{0};
            size_t size_{0};
            size_t total_recorded_{0};
            mutable std::mutex mutex_;
    };

}

#endif

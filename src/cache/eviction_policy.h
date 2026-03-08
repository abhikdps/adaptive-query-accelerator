#ifndef ADAPTIVE_QUERY_ACCELERATOR_EVICTION_POLICY_H
#define ADAPTIVE_QUERY_ACCELERATOR_EVICTION_POLICY_H

#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace aqa {

    class AccessObserver;

    class PageEvictionPolicy {
        public:
            virtual ~PageEvictionPolicy() = default;
            virtual uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) = 0;
    };

    class LruPageEvictionPolicy : public PageEvictionPolicy {
        public:
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
    };

    class ScanResistantPageEvictionPolicy : public PageEvictionPolicy {
        public:
            explicit ScanResistantPageEvictionPolicy(AccessObserver* observer);
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
        private:
            AccessObserver* observer_;
    };

    class LfuPageEvictionPolicy : public PageEvictionPolicy {
        public:
            explicit LfuPageEvictionPolicy(AccessObserver* observer);
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
        private:
            AccessObserver* observer_;
    };

    class ScanResistantThenLfuPageEvictionPolicy : public PageEvictionPolicy {
        public:
            explicit ScanResistantThenLfuPageEvictionPolicy(AccessObserver* observer);
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
        private:
            AccessObserver* observer_;
    };

    class RecordEvictionPolicy {
        public:
            virtual ~RecordEvictionPolicy() = default;
            virtual size_t choose_victim(const std::vector<const std::vector<uint8_t>*>& keys_lru_order) = 0;
    };

    class LruRecordEvictionPolicy : public RecordEvictionPolicy {
        public:
            size_t choose_victim(const std::vector<const std::vector<uint8_t>*>& keys_lru_order) override;
    };

}

#endif

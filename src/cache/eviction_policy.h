#ifndef ADAPTIVE_QUERY_ACCELERATOR_EVICTION_POLICY_H
#define ADAPTIVE_QUERY_ACCELERATOR_EVICTION_POLICY_H

#include <cstddef>
#include <cstdint>
#include <optional>
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

    class HintAwarePageEvictionPolicy : public PageEvictionPolicy {
        public:
            explicit HintAwarePageEvictionPolicy(AccessObserver* observer);
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
        private:
            AccessObserver* observer_;
    };

    class LearnedPageEvictionPolicy : public PageEvictionPolicy {
        public:
            explicit LearnedPageEvictionPolicy(AccessObserver* observer,
                                              size_t feedback_horizon = 64,
                                              double learning_rate = 0.05);
            uint32_t choose_victim(const std::vector<uint32_t>& unpinned_page_ids_lru_order) override;
        private:
            struct Features { double recency{0}, count{0}, scan{0}; };
            Features extract_features(uint32_t page_id) const;
            double score(const Features& f) const;
            void update_weights(const Features& f, int label);

            AccessObserver* observer_;
            size_t feedback_horizon_;
            double learning_rate_;
            double w_recency_{1.0}, w_count_{1.0}, w_scan_{-1.0};
            std::optional<uint32_t> last_evicted_page_id_;
            Features last_evicted_features_;
            size_t last_eviction_total_{0};
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

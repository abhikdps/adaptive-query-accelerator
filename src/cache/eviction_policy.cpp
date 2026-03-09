#include "cache/eviction_policy.h"
#include "observer/access_observer.h"
#include "workload_hint.h"

namespace aqa {

    uint32_t LruPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        return unpinned_page_ids_lru_order.empty() ? 0 : unpinned_page_ids_lru_order.front();
    }

    ScanResistantPageEvictionPolicy::ScanResistantPageEvictionPolicy(AccessObserver* observer)
        : observer_(observer) {}

    uint32_t ScanResistantPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        if (unpinned_page_ids_lru_order.empty()) return 0;
        if (!observer_) return unpinned_page_ids_lru_order.front();

        std::vector<uint32_t> recent = observer_->get_recent_page_ids(observer_->get_capacity());
        std::unordered_set<uint32_t> recent_set(recent.begin(), recent.end());

        for (uint32_t pid : unpinned_page_ids_lru_order) {
            bool in_run = (recent_set.count(pid + 1) != 0) || (pid > 0 && recent_set.count(pid - 1) != 0);
            if (in_run) return pid;
        }
        return unpinned_page_ids_lru_order.front();
    }

    LfuPageEvictionPolicy::LfuPageEvictionPolicy(AccessObserver* observer)
        : observer_(observer) {}

    uint32_t LfuPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        if (unpinned_page_ids_lru_order.empty()) return 0;
        if (!observer_) return unpinned_page_ids_lru_order.front();

        uint32_t victim = unpinned_page_ids_lru_order.front();
        uint64_t min_count = observer_->get_access_count(victim);
        for (size_t i = 1; i < unpinned_page_ids_lru_order.size(); ++i) {
            uint32_t pid = unpinned_page_ids_lru_order[i];
            uint64_t c = observer_->get_access_count(pid);
            if (c < min_count) {
                min_count = c;
                victim = pid;
            }
        }
        return victim;
    }

    ScanResistantThenLfuPageEvictionPolicy::ScanResistantThenLfuPageEvictionPolicy(AccessObserver* observer)
        : observer_(observer) {}

    uint32_t ScanResistantThenLfuPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        if (unpinned_page_ids_lru_order.empty()) return 0;
        if (!observer_) return unpinned_page_ids_lru_order.front();

        std::vector<uint32_t> recent = observer_->get_recent_page_ids(observer_->get_capacity());
        std::unordered_set<uint32_t> recent_set(recent.begin(), recent.end());

        std::vector<uint32_t> scan_candidates;
        for (uint32_t pid : unpinned_page_ids_lru_order) {
            bool in_run = (recent_set.count(pid + 1) != 0) || (pid > 0 && recent_set.count(pid - 1) != 0);
            if (in_run) scan_candidates.push_back(pid);
        }

        const std::vector<uint32_t>& candidates = scan_candidates.empty()
            ? unpinned_page_ids_lru_order : scan_candidates;

        uint32_t victim = candidates.front();
        uint64_t min_count = observer_->get_access_count(victim);
        for (size_t i = 1; i < candidates.size(); ++i) {
            uint32_t pid = candidates[i];
            uint64_t c = observer_->get_access_count(pid);
            if (c < min_count) {
                min_count = c;
                victim = pid;
            }
        }
        return victim;
    }

    HintAwarePageEvictionPolicy::HintAwarePageEvictionPolicy(AccessObserver* observer)
        : observer_(observer) {}

    uint32_t HintAwarePageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        if (unpinned_page_ids_lru_order.empty()) return 0;
        if (get_workload_hint() == WorkloadHint::Scan) {
            if (!observer_) return unpinned_page_ids_lru_order.front();
            std::vector<uint32_t> recent = observer_->get_recent_page_ids(observer_->get_capacity());
            std::unordered_set<uint32_t> recent_set(recent.begin(), recent.end());
            for (uint32_t pid : unpinned_page_ids_lru_order) {
                bool in_run = (recent_set.count(pid + 1) != 0) || (pid > 0 && recent_set.count(pid - 1) != 0);
                if (in_run) return pid;
            }
            return unpinned_page_ids_lru_order.front();
        }
        if (!observer_) return unpinned_page_ids_lru_order.front();
        uint32_t victim = unpinned_page_ids_lru_order.front();
        uint64_t min_count = observer_->get_access_count(victim);
        for (size_t i = 1; i < unpinned_page_ids_lru_order.size(); ++i) {
            uint32_t pid = unpinned_page_ids_lru_order[i];
            uint64_t c = observer_->get_access_count(pid);
            if (c < min_count) {
                min_count = c;
                victim = pid;
            }
        }
        return victim;
    }

    LearnedPageEvictionPolicy::LearnedPageEvictionPolicy(AccessObserver* observer,
                                                          size_t feedback_horizon,
                                                          double learning_rate)
        : observer_(observer), feedback_horizon_(feedback_horizon), learning_rate_(learning_rate) {}

    LearnedPageEvictionPolicy::Features LearnedPageEvictionPolicy::extract_features(uint32_t page_id) const {
        Features f;
        if (!observer_) return f;
        size_t cap = observer_->get_capacity();
        size_t last_idx = observer_->get_last_index_in_recent(page_id);
        f.recency = 1.0 / (1.0 + static_cast<double>(last_idx));
        uint64_t cnt = observer_->get_access_count(page_id);
        f.count = std::min(1.0, static_cast<double>(cnt) / (1.0 + static_cast<double>(cap)));
        std::vector<uint32_t> recent = observer_->get_recent_page_ids(cap);
        std::unordered_set<uint32_t> recent_set(recent.begin(), recent.end());
        f.scan = ((recent_set.count(page_id + 1) != 0) || (page_id > 0 && recent_set.count(page_id - 1) != 0)) ? 1.0 : 0.0;
        return f;
    }

    double LearnedPageEvictionPolicy::score(const Features& f) const {
        return w_recency_ * f.recency + w_count_ * f.count + w_scan_ * f.scan;
    }

    void LearnedPageEvictionPolicy::update_weights(const Features& f, int label) {
        double delta = learning_rate_ * (2 * label - 1);
        w_recency_ += delta * f.recency;
        w_count_ += delta * f.count;
        w_scan_ += delta * f.scan;
    }

    uint32_t LearnedPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        if (unpinned_page_ids_lru_order.empty()) return 0;
        if (!observer_) return unpinned_page_ids_lru_order.front();

        if (last_evicted_page_id_.has_value()) {
            size_t total_now = observer_->get_total_recorded();
            if (total_now >= last_eviction_total_ + feedback_horizon_) {
                size_t idx = observer_->get_last_index_in_recent(*last_evicted_page_id_);
                int label = (idx < observer_->get_capacity()) ? 1 : 0;
                update_weights(last_evicted_features_, label);
                last_evicted_page_id_.reset();
            }
        }

        uint32_t victim = unpinned_page_ids_lru_order.front();
        Features victim_features = extract_features(victim);
        double min_score = score(victim_features);

        for (size_t i = 1; i < unpinned_page_ids_lru_order.size(); ++i) {
            uint32_t pid = unpinned_page_ids_lru_order[i];
            Features f = extract_features(pid);
            double s = score(f);
            if (s < min_score) {
                min_score = s;
                victim = pid;
                victim_features = f;
            }
        }

        last_evicted_page_id_ = victim;
        last_evicted_features_ = victim_features;
        last_eviction_total_ = observer_->get_total_recorded();
        return victim;
    }

    size_t LruRecordEvictionPolicy::choose_victim(
        const std::vector<const std::vector<uint8_t>*>& keys_lru_order) {
        (void)keys_lru_order;
        return 0;
    }

}

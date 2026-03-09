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

    size_t LruRecordEvictionPolicy::choose_victim(
        const std::vector<const std::vector<uint8_t>*>& keys_lru_order) {
        (void)keys_lru_order;
        return 0;
    }

}

#include "cache/lru_cache.h"
#include "cache/eviction_policy.h"
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

namespace aqa {
    LruCache::LruCache(size_t capacity, RecordEvictionPolicy* eviction_policy)
        : capacity_(capacity), eviction_policy_(eviction_policy) {}

    std::optional<std::vector<uint8_t>> LruCache::get(const std::vector<uint8_t>& key) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = map_.find(key);
        if ( it == map_.end()) {
            misses_++;
            return std::nullopt;
        }

        hits_++;
        list_.splice(list_.begin(), list_, it->second);

        return it->second->value;
    }

    void LruCache::put(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it != map_.end()) {
            list_.splice(list_.begin(), list_, it->second);
            it->second->value = value;
            return;
        }

        if (list_.size() >= capacity_) {
            std::vector<ListIterator> iters_lru;
            iters_lru.reserve(list_.size());
            for (auto it = list_.end(); it != list_.begin(); ) {
                --it;
                iters_lru.push_back(it);
            }
            std::vector<const std::vector<uint8_t>*> key_ptrs;
            key_ptrs.reserve(iters_lru.size());
            for (const auto& it : iters_lru) {
                key_ptrs.push_back(&it->key);
            }
            size_t victim_idx = eviction_policy_
                ? eviction_policy_->choose_victim(key_ptrs)
                : 0;
            if (victim_idx >= iters_lru.size()) {
                victim_idx = 0;
            }
            auto victim_it = iters_lru[victim_idx];
            map_.erase(victim_it->key);
            list_.erase(victim_it);
        }

        list_.push_front({key, value});
        map_[key] = list_.begin();
    }
}

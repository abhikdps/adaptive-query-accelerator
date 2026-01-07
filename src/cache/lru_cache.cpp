#include "cache/lru_cache.h"
#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

namespace aqa {
    LruCache::LruCache(size_t capacity) : capacity_(capacity) {}

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
            auto last = list_.end();
            last--;
            map_.erase(last->key);
            list_.pop_back();
        }

        list_.push_front({key, value});
        map_[key] = list_.begin();
    }
}

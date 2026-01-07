#ifndef ADAPTIVE_QUERY_ACCELERATOR_LRU_CACHE_H
#define ADAPTIVE_QUERY_ACCELERATOR_LRU_CACHE_H

#include <cstdint>
#include <vector>
#include <list>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "index/index.h"

namespace aqa {
    class LruCache {
        public:
            explicit LruCache(size_t capacity);

            std::optional<std::vector<uint8_t>> get(const std::vector<uint8_t>& key);

            void put(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value);

            size_t get_hits() const { return hits_; }
            size_t get_misses() const { return misses_; }

        private:
            size_t capacity_;
            size_t hits_ = 0;
            size_t misses_ = 0;

            struct CacheEntry {
                std::vector<uint8_t> key;
                std::vector<uint8_t> value;
            };
            std::list<CacheEntry> list_;

            using ListIterator = std::list<CacheEntry>::iterator;
            std::unordered_map<std::vector<uint8_t>, ListIterator, VectorHash> map_;

            mutable std::mutex mutex_;
    };
}

#endif

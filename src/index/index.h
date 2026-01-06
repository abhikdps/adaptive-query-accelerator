#ifndef ADAPTIVE_QUERY_ACCELERATOR_INDEX_H
#define ADAPTIVE_QUERY_ACCELERATOR_INDEX_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <optional>
#include "storage/page.h"

namespace aqa {
    class StorageReader;
}

namespace aqa {
    struct VectorHash {
        std::size_t operator()(const std::vector<uint8_t>& k) const {
            std::size_t seed = 0;
            for(uint8_t b : k) {
                seed ^= std::hash<uint8_t>{}(b) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };

    class Index {
        public:
            Index() = default;

            void insert(const std::vector<uint8_t>& key, RecordID rid);

            std::optional<RecordID> lookup(const std::vector<uint8_t>& key) const;

            void rebuild(StorageReader& reader);

            size_t size() const { return map_.size(); }

        private:
            std::unordered_map<std::vector<uint8_t>, RecordID, VectorHash> map_;
    };
}

#endif

#include "index/index.h"
#include "storage/page.h"
#include "storage/storage_reader.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

namespace aqa {
    void Index::insert(const std::vector<uint8_t>& key, RecordID rid) {
        map_[key] = rid;
    }

    std::optional<RecordID> Index::lookup(const std::vector<uint8_t>& key) const {
        auto it = map_.find(key);
        if (it != map_.end()) {
            return  it->second;
        }
        return std::nullopt;
    }

    void Index::rebuild(StorageReader& reader) {
        std::cout << "Rebuilding index from disk..." << std::endl;
        map_.clear();

        reader.scan([this](RecordID rid, const std::vector<uint8_t>& key, const std::vector<uint8_t>& /*value*/) {
            this->insert(key, rid);
        });

        std::cout << "Index rebuild complete. Loaded " << map_.size() << " keys." << std::endl;
    }
}

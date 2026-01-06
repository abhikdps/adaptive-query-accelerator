#ifndef ADAPTIVE_QUERY_ACCELERATOR_STORAGE_READER_H
#define ADAPTIVE_QUERY_ACCELERATOR_STORAGE_READER_H

#include "storage/storage_engine.h"
#include "storage/page.h"
#include <cstdint>
#include <vector>
#include <optional>
#include <functional>

namespace aqa {
    struct Record {
        std::vector<uint8_t> key;
        std::vector<uint8_t> value;
    };

    class StorageReader {
        public:
            explicit StorageReader(StorageEngine& engine);

            std::optional<Record> read_record(RecordID rid);

            void scan(std::function<void(RecordID, const std::vector<uint8_t>&, const std::vector<uint8_t>&)> callback);

        private:
            StorageEngine& engine_;
    };
}

#endif

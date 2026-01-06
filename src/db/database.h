#ifndef ADAPTIVE_QUERY_ACCELERATOR_DATABASE_H
#define ADAPTIVE_QUERY_ACCELERATOR_DATABASE_H

#include "storage/storage_engine.h"
#include "storage/storage_reader.h"
#include "storage/storage_writer.h"
#include "index/index.h"
#include <cstdint>
#include <string>
#include <memory>
#include <optional>
#include <vector>

namespace aqa {
    class Database {
        public:
            explicit Database(const std::string& path, size_t cache_size_pages = 1000);

            void put(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value);

            std::optional<std::vector<uint8_t>> get(const std::vector<uint8_t>& key);

            double get_recovery_time_ms() const { return recovery_time_ms_; }
            size_t get_record_count() const { return index_.size(); }

        private:
            void recover();

            std::string path_;

            std::unique_ptr<StorageEngine> engine_;
            std::unique_ptr<StorageWriter> writer_;
            std::unique_ptr<StorageReader> reader_;

            Index index_;

            double recovery_time_ms_ = 0.0;
    };
}

#endif

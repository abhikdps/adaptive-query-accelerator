#include "db/database.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <ratio>
#include <shared_mutex>
#include <vector>
#include "storage/page.h"
#include "storage/storage_engine.h"
#include "storage/storage_reader.h"
#include "storage/storage_writer.h"

namespace aqa {
    Database::Database(const std::string& path, size_t cache_size_pages, size_t record_cache_size)
        : path_(path), record_cache_(record_cache_size) {
            engine_ = std::make_unique<StorageEngine>(path, cache_size_pages);
            reader_ = std::make_unique<StorageReader>(*engine_);
            writer_ = std::make_unique<StorageWriter>(path);

            recover();
        }

    void Database::recover() {
        auto start = std::chrono::high_resolution_clock::now();

        if (engine_->get_total_pages() > 0) {
            std::cout<< "[Database] Starting recovery scan.." <<std::endl;

            index_.rebuild(*reader_);
        }

        auto end = std::chrono::high_resolution_clock::now();
        recovery_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "[Database] Recovery complete. "
                  << index_.size() << " keys loaded in "
                  << recovery_time_ms_ << " ms" << std::endl;
    }

    void Database::put(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
        std::unique_lock<std::shared_mutex> lock(rw_mutex_);

        RecordID rid = writer_->append(key, value);
        index_.insert(key, rid);

        record_cache_.put(key, value);
    }

    std::optional<std::vector<uint8_t>> Database::get(const std::vector<uint8_t>& key) {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);

        if (auto cached_val = record_cache_.get(key)) {
            return cached_val;
        }

        auto rid_opt = index_.lookup(key);
        if (!rid_opt) return std::nullopt;

        auto record_opt = reader_->read_record(*rid_opt);
        if (record_opt && record_opt->key == key) {
            record_cache_.put(key, record_opt->value);
            return record_opt->value;
        }

        return std::nullopt;
    }

    size_t Database::get_record_count() const {
        std::shared_lock<std::shared_mutex> lock(rw_mutex_);
        return index_.size();
    }
}

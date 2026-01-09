#include "wal/wal_manager.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <vector>

namespace aqa {
    WalManager::WalManager(const std::string& wal_path) : path_(wal_path) {
        log_file_.open(path_, std::ios::binary | std::ios::app);
    }

    WalManager::~WalManager() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    uint32_t WalManager::calculate_checksum(uint64_t lsn, const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
        uint32_t checksum = 0;

        auto mix = [&](uint64_t v) {
            checksum ^= (v & 0xFFFFFFFF);
            checksum ^= (v >> 32);
        };

        mix(lsn);
        for(uint8_t b : key) checksum += b;
        for(uint8_t b : value) checksum +=b;

        return checksum;
    }

    uint64_t WalManager::append(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value) {
        std::lock_guard<std::mutex> lock(mutex_);

        current_lsn_++;

        uint32_t checksum = calculate_checksum(current_lsn_, key, value);
        uint16_t k_len = static_cast<uint16_t>(key.size());
        uint16_t v_len = static_cast<uint16_t>(value.size());

        log_file_.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
        log_file_.write(reinterpret_cast<const char*>(&current_lsn_), sizeof(current_lsn_));
        log_file_.write(reinterpret_cast<const char*>(&k_len), sizeof(k_len));
        log_file_.write(reinterpret_cast<const char*>(&v_len), sizeof(v_len));

        if (k_len > 0) log_file_.write(reinterpret_cast<const char*>(key.data()), k_len);
        if (v_len > 0) log_file_.write(reinterpret_cast<const char*>(value.data()), v_len);

        log_file_.flush();

        return current_lsn_;
    }

    size_t WalManager::replay(std::function<void(const WalRecord&)> visitor) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_.close();

        std::ifstream reader(path_, std::ios::binary);
        if (!reader.is_open()) {
            log_file_.open(path_, std::ios::binary | std::ios::app);
            return 0;
        }

        size_t count = 0;
        while (reader.peek() != EOF) {
            uint32_t stored_checksum;
            uint64_t lsn;
            uint16_t k_len, v_len;

            reader.read(reinterpret_cast<char*>(&stored_checksum), sizeof(stored_checksum));
            reader.read(reinterpret_cast<char*>(&lsn), sizeof(lsn));
            reader.read(reinterpret_cast<char*>(&k_len), sizeof(k_len));
            reader.read(reinterpret_cast<char*>(&v_len), sizeof(v_len));

            if (reader.eof()) break;

            std::vector<uint8_t> key(k_len);
            std::vector<uint8_t> value(v_len);

            if (k_len > 0) reader.read(reinterpret_cast<char*>(key.data()), k_len);
            if (v_len > 0) reader.read(reinterpret_cast<char*>(value.data()), v_len);

            if (reader.fail()) break;

            uint32_t calc_checksum = calculate_checksum(lsn, key, value);
            if (calc_checksum != stored_checksum) {
                std::cerr << "[WAL] Corruption detected at LSN " << lsn << ". Stopping replay." << std::endl;
                break;
            }

            visitor({lsn, key, value});
            if (lsn > current_lsn_) current_lsn_ = lsn;
            count++;
        }

        reader.close();
        log_file_.open(path_, std::ios::binary | std::ios::app);
        return count;
    }

    void WalManager::truncate() {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_.close();
        log_file_.open(path_, std::ios::binary | std::ios::trunc);
        current_lsn_ = 0;
    }
}

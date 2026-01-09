#ifndef ADAPTIVE_QUERY_ACCELERATOR_WAL_MANAGER_H
#define ADAPTIVE_QUERY_ACCELERATOR_WAL_MANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <cstdint>
#include <functional>

namespace aqa {
    struct WalRecord {
        uint64_t lsn;
        std::vector<uint8_t> key;
        std::vector<uint8_t> value;
    };

    class WalManager {
        public:
            explicit WalManager(const std::string& wal_path);
            ~WalManager();

            uint64_t append(const std::vector<uint8_t>& key, const std::vector<uint8_t>& value);

            size_t replay(std::function<void(const WalRecord&)> visitor);

            void truncate();

        private:
            uint32_t calculate_checksum(uint64_t lsn, const std::vector<uint8_t>& key, const std::vector<uint8_t>& value);

            std::string path_;
            std::ofstream log_file_;
            std::mutex mutex_;
            uint64_t current_lsn_ = 0;
    };
}

#endif

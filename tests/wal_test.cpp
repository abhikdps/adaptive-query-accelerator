#include "wal/wal_manager.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }

using namespace aqa;

std::vector<uint8_t> bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_wal_write_and_replay() {
    std::string path = "test_wal.log";
    std::filesystem::remove(path);

    {
        WalManager wal(path);
        wal.append(bytes("key1"), bytes("value1"));
        wal.append(bytes("key2"), bytes("value2"));
        wal.append(bytes("key3"), bytes("value3"));
    }

    {
        WalManager wal(path);
        std::vector<WalRecord> records;

        size_t count = wal.replay([&](const WalRecord& rec) {
            records.push_back(rec);
        });

        ASSERT_EQ(count, 3);
        ASSERT_EQ(records.size(), 3);

        ASSERT_EQ(records[0].lsn, 1);
        ASSERT_EQ(records[1].lsn, 2);
        ASSERT_EQ(records[2].lsn, 3);

        auto k1 = bytes("key1");
        auto v3 = bytes("value3");

        if (records[0].key != k1) {
             std::cerr << "FAIL: Key mismatch on record 0" << std::endl; exit(1);
        }
        if (records[2].value != v3) {
             std::cerr << "FAIL: Value mismatch on record 2" << std::endl; exit(1);
        }
    }

    std::cout << "[Pass] WAL Write & Replay" << std::endl;
    std::filesystem::remove(path);
}

void test_wal_truncation() {
    std::string path = "test_wal_trunc.log";
    std::filesystem::remove(path);

    {
        WalManager wal(path);
        wal.append(bytes("temp"), bytes("data"));
    }

    {
        WalManager wal(path);
        wal.truncate();

        size_t count = wal.replay([](const WalRecord&){});
        ASSERT_EQ(count, 0);
    }

    ASSERT_EQ(std::filesystem::file_size(path), 0);

    std::cout << "[Pass] WAL Truncation" << std::endl;
    std::filesystem::remove(path);
}

int main() {
    test_wal_write_and_replay();
    test_wal_truncation();
    return 0;
}

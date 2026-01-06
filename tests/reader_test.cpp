#include "storage/storage_writer.h"
#include "storage/storage_reader.h"
#include "storage/storage_engine.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }

std::vector<uint8_t> str_to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::string bytes_to_str(const std::vector<uint8_t>& b) {
    return std::string(b.begin(), b.end());
}

void test_read_write_cycle() {
    std::string path = "rw_test.db";
    std::filesystem::remove(path);

    std::cout << "Step 1: Writing data.." << std::endl;
    {
        aqa::StorageWriter writer(path);
        for(int i = 0; i < 100; ++i) {
            writer.append(str_to_bytes("key_" + std::to_string(i)),
                        str_to_bytes("val_" + std::to_string(i)));
        }
    }

    std::cout << "Step 2: Reading data.." << std::endl;
    {
        aqa::StorageEngine engine(path, 10);
        aqa::StorageReader reader(engine);

        int count = 0;
        reader.scan([&](aqa::RecordID rid, const std::vector<uint8_t>& k, const std::vector<uint8_t>& v) {
            (void)rid;

            std::string key_str = bytes_to_str(k);
            std::string val_str = bytes_to_str(v);

            std::string suffix = key_str.substr(4);
            std::string expected_val = "val_" + suffix;

            ASSERT_EQ(val_str, expected_val);
            count++;
        });

        ASSERT_EQ(count, 100);
    }

    std::cout << "[Pass] Read/Write Cycle Integration" << std::endl;
    std::filesystem::remove(path);
}

int main() {
    test_read_write_cycle();
    return 0;
}

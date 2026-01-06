#include "storage/storage_writer.h"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cstring>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }

std::vector<uint8_t> to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_writer() {
    std::string path = "writer_test.db";
    std::filesystem::remove(path);

    {
        aqa::StorageWriter writer(path);

        for(int i = 0; i < 5; ++i) {
            auto rid = writer.append(to_bytes("key" + std::to_string(i)), to_bytes("val" + std::to_string(i)));
            ASSERT_EQ(rid.page_id, 0);
            ASSERT_EQ(rid.slot_id, i);
        }

        std::vector<uint8_t> big_data(2000, 'X');

        auto rid_big = writer.append(to_bytes("big1"), big_data);
        ASSERT_EQ(rid_big.page_id, 0);

        auto rid_overflow = writer.append(to_bytes("big2"), big_data);
        ASSERT_EQ(rid_overflow.page_id, 1);
        ASSERT_EQ(rid_overflow.slot_id, 0);
    }

    size_t file_size = std::filesystem::file_size(path);
    ASSERT_EQ(file_size, 8192);

    std::cout << "[Pass] Storage Writer Test" << std::endl;
    std::filesystem::remove(path);
}

int main () {
    test_writer();
    return 0;
}

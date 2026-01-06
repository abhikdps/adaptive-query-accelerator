#include "index/index.h"
#include "storage/page.h"
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
#define ASSERT_TRUE(a) if (!(a)) { std::cerr << "FAIL: Expression false" << std::endl; exit(1); }

std::vector<uint8_t> to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_memory_index() {
    aqa::Index idx;

    std::vector<uint8_t> key1 = to_bytes("user:100");
    aqa::RecordID rid1 = {1, 5};

    idx.insert(key1, rid1);

    auto result = idx.lookup(key1);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->page_id, 1);
    ASSERT_EQ(result->slot_id, 5);

    aqa::RecordID rid2 = {2, 0};
    idx.insert(key1, rid2);

    result = idx.lookup(key1);
    ASSERT_EQ(result->page_id, 2);

    auto missing = idx.lookup(to_bytes("ghost"));
    ASSERT_TRUE(!missing.has_value());

    std::cout << "[Pass] Basic Index Operations" <<std::endl;
}

void test_index_rebuild() {
    std::string path = "index_test.db";
    std::filesystem::remove(path);

    {
        aqa::StorageWriter writer(path);
        writer.append(to_bytes("apple"), to_bytes("red"));
        writer.append(to_bytes("banana"), to_bytes("yellow"));
        writer.append(to_bytes("cherry"), to_bytes("red"));
    }

    {
        aqa::StorageEngine engine(path, 10);
        aqa::StorageReader reader(engine);
        aqa::Index idx;

        idx.rebuild(reader);
        ASSERT_EQ(idx.size(), 3);

        auto loc = idx.lookup(to_bytes("banana"));
        ASSERT_TRUE(loc.has_value());
        ASSERT_EQ(loc->page_id, 0);
        ASSERT_EQ(loc->slot_id, 1);
    }

    std::cout << "[Pass] Index Rebuild from Disk" << std::endl;
    std::filesystem::remove(path);
}

int main() {
    test_memory_index();
    test_index_rebuild();
    return 0;
}

#include "storage/storage_engine.h"
#include "storage/storage_reader.h"
#include "storage/storage_writer.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b << std::endl; \
        exit(1); \
    }
#define ASSERT_TRUE(c) \
    if (!(c)) { \
        std::cerr << "FAIL: " << #c << std::endl; \
        exit(1); \
    }

static std::vector<uint8_t> str_to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_prefetch_page_bounds_check() {
    std::string path = "prefetch_bounds.db";
    std::filesystem::remove(path);
    {
        aqa::StorageWriter w(path);
        w.append(str_to_bytes("k"), str_to_bytes("v"));
    }
    aqa::StorageEngine engine(path, 4);
    engine.prefetch_page(0);
    engine.prefetch_page(1);
    engine.prefetch_page(1000);
    auto h = engine.fetch_page(0);
    ASSERT_TRUE(h->get_header().magic == 0x41514144u);
    std::filesystem::remove(path);
}

void test_scan_with_prefetch() {
    std::string path = "prefetch_scan.db";
    std::filesystem::remove(path);
    {
        aqa::StorageWriter w(path);
        for (int i = 0; i < 20; ++i) {
            w.append(str_to_bytes("key_" + std::to_string(i)),
                     str_to_bytes("val_" + std::to_string(i)));
        }
    }
    aqa::StorageEngine engine(path, 32);
    aqa::StorageReader reader(engine);
    size_t count = 0;
    reader.scan([&](aqa::RecordID, const std::vector<uint8_t>&, const std::vector<uint8_t>&) {
        count++;
    });
    ASSERT_EQ(count, 20u);
    std::filesystem::remove(path);
}

int main() {
    test_prefetch_page_bounds_check();
    test_scan_with_prefetch();
    std::cout << "All prefetch tests passed." << std::endl;
    return 0;
}

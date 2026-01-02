#include "storage/storage_engine.h"
#include <iostream>
#include <filesystem>
#include <cassert>
#include <cstring>

#define ASSERT_EQ(a, b) \
    if((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; \
        exit(1); \
    }

void test_full_stack() {
    std::string path = "integration.db";
    std::filesystem::remove(path);

    {
        aqa::StorageEngine db(path, 5);

        for (uint32_t i = 0; i < 10; ++i) {
            auto h = db.allocate_page();
            h->get_header_mut().page_id = i;

            auto span = h->get_payload_mut();
            uint32_t* data = reinterpret_cast<uint32_t*>(span.data());
            data[0] = 0xDEADBEEF + i;
        }

        db.flush_all();
    }

    {
        aqa::StorageEngine db(path, 5);
        ASSERT_EQ(db.get_total_pages(), 10);

        auto h0 = db.fetch_page(0);
        ASSERT_EQ(h0->get_header().page_id, 0);

        auto span = h0->get_payload();
        const uint32_t* data = reinterpret_cast<const uint32_t*>(span.data());

        if (data[0] != 0xDEADBEEF) {
            std::cout << "Debug: Page 0 Data is: " << std::hex << data[0] << std::dec << std::endl;
        }
        ASSERT_EQ(data[0], 0xDEADBEEF + 0);

        if (db.get_cache_misses() == 0) {
             std::cerr << "Warning: Expected cache miss for Page 0\n";
        }
    }

    std::filesystem::remove(path);
    std::cout << "[Pass] Integration Test" << std::endl;
}

int main() {
    test_full_stack();
    return 0;
}

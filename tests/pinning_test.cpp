#include "storage/page_cache.h"
#include <iostream>
#include <filesystem>
#include <cassert>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }
#define ASSERT_THROWS(expr) { bool caught = false; try {expr;} catch(...) { caught = true; } if (!caught) { std::cerr << "Test Failed: Expected exception for " << #expr << std::endl; exit(1); } }

void test_pinning() {
    std::string test_file = "pin_test.db";
    std::filesystem::remove(test_file);

    aqa::MappedFile file(test_file);
    file.grow_file(10);

    aqa::PageCache cache(file, 2);

    {
        auto h0 = cache.fetch_page(0);
        ASSERT_EQ(cache.get_pin_count_for_test(0), 1);

        {
            auto h1 = cache.fetch_page(1);
            ASSERT_EQ(cache.get_pin_count_for_test(1), 1);
            ASSERT_EQ(cache.get_size(), 2);

            ASSERT_THROWS(cache.fetch_page(2));
        }

        ASSERT_EQ(cache.get_pin_count_for_test(1), 0);
        ASSERT_EQ(cache.get_pin_count_for_test(0), 1);

        auto h2 = cache.fetch_page(2);
        ASSERT_EQ(cache.get_size(), 2);
    }

    ASSERT_EQ(cache.get_pin_count_for_test(0), 0);

    std::cout << "[Pass] Pinning Logic Test" << std::endl;
    std::filesystem::remove(test_file);
}

int main() {
    test_pinning();
    return 0;
}

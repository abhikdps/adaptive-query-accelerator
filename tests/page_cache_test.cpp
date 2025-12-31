#include "storage/page_cache.h"
#include <iostream>
#include <filesystem>
#include <cassert>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "Test failed: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" <<std::endl;  exit(1); }

void test_lru_eviction() {
    std::string test_file = "cache_test.db";
    std::filesystem::remove(test_file);

    aqa::MappedFile file(test_file);
    file.grow_file(10);

    aqa::PageCache cache(file, 2);

    cache.get_page(0);
    cache.get_page(1);

    ASSERT_EQ(cache.get_size(), 2);
    ASSERT_EQ(cache.get_misses(), 2);

    cache.get_page(0);
    ASSERT_EQ(cache.get_hits(), 1);

    cache.get_page(2);

    size_t misses_before = cache.get_misses();
    cache.get_page(1);
    size_t misses_after = cache.get_misses();

    ASSERT_EQ(misses_after, misses_before + 1);

    std::cout << "[Pass] LRU Eviction test" << std::endl;
    std::filesystem::remove(test_file);
}

int main() {
    test_lru_eviction();
    return 0;
}

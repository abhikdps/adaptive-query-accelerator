#include "cache/eviction_policy.h"
#include "storage/page_cache.h"
#include "storage/mapped_file.h"
#include "db/database.h"
#include <filesystem>
#include <iostream>
#include <vector>

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b << std::endl; \
        exit(1); \
    }

void test_lru_page_eviction_policy() {
    aqa::LruPageEvictionPolicy policy;
    std::vector<uint32_t> unpinned = {10, 20, 30};
    uint32_t victim = policy.choose_victim(unpinned);
    ASSERT_EQ(victim, 10u);
}

void test_lru_record_eviction_policy() {
    aqa::LruRecordEvictionPolicy policy;
    std::vector<uint8_t> k1 = {1};
    std::vector<uint8_t> k2 = {2};
    std::vector<const std::vector<uint8_t>*> keys = {&k1, &k2};
    size_t idx = policy.choose_victim(keys);
    ASSERT_EQ(idx, 0u);
}

void test_page_cache_with_explicit_lru_policy() {
    std::string path = "eviction_page.db";
    std::filesystem::remove(path);
    aqa::MappedFile file(path);
    file.grow_file(4);
    aqa::LruPageEvictionPolicy lru_policy;
    aqa::PageCache cache(file, 2, nullptr, &lru_policy);
    {
        auto h0 = cache.fetch_page(0);
        auto h1 = cache.fetch_page(1);
        (void)h0;
        (void)h1;
    }
    auto h2 = cache.fetch_page(2);
    (void)h2;
    ASSERT_EQ(cache.get_size(), 2u);
    std::filesystem::remove(path);
}

void test_record_cache_with_explicit_lru_policy() {
    aqa::LruRecordEvictionPolicy policy;
    aqa::LruCache cache(2, &policy);
    std::vector<uint8_t> k1 = {'a'};
    std::vector<uint8_t> k2 = {'b'};
    std::vector<uint8_t> k3 = {'c'};
    std::vector<uint8_t> v = {'v'};
    cache.put(k1, v);
    cache.put(k2, v);
    cache.put(k3, v);
    auto r = cache.get(k1);
    ASSERT_EQ(r.has_value(), false);
    r = cache.get(k2);
    ASSERT_EQ(r.has_value(), true);
    r = cache.get(k3);
    ASSERT_EQ(r.has_value(), true);
}

int main() {
    test_lru_page_eviction_policy();
    test_lru_record_eviction_policy();
    test_page_cache_with_explicit_lru_policy();
    test_record_cache_with_explicit_lru_policy();
    std::cout << "All eviction policy tests passed." << std::endl;
    return 0;
}

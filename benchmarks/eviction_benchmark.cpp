#include "storage/page_cache.h"
#include "storage/mapped_file.h"
#include "observer/access_observer.h"
#include "cache/eviction_policy.h"
#include "workload_hint.h"
#include <filesystem>
#include <iostream>
#include <vector>

using namespace aqa;

static const size_t CACHE_CAPACITY = 32;
static const size_t WORKING_SET_PAGES = 8;
static const size_t SCAN_LEAD_PAGES = 16;
static const size_t SCAN_TAIL_PAGES = 80;
static const size_t REPEAT_WORKING_SET = 4;
static const std::string BENCH_PATH = "eviction_bench.db";

static size_t total_scan_pages() {
    return SCAN_LEAD_PAGES + SCAN_TAIL_PAGES;
}

void run_workload(PageCache& cache, AccessObserver& /*observer*/,
                 bool use_scan_hint_during_scan) {
    for (size_t r = 0; r < REPEAT_WORKING_SET; ++r) {
        for (size_t p = 0; p < WORKING_SET_PAGES; ++p) {
            auto h = cache.fetch_page(static_cast<uint32_t>(p));
            (void)h;
        }
    }
    for (size_t p = WORKING_SET_PAGES; p < WORKING_SET_PAGES + SCAN_LEAD_PAGES; ++p) {
        auto h = cache.fetch_page(static_cast<uint32_t>(p));
        (void)h;
    }
    for (size_t r = 0; r < REPEAT_WORKING_SET; ++r) {
        for (size_t p = 0; p < WORKING_SET_PAGES; ++p) {
            auto h = cache.fetch_page(static_cast<uint32_t>(p));
            (void)h;
        }
    }
    if (use_scan_hint_during_scan) {
        set_workload_hint(WorkloadHint::Scan);
    }
    for (size_t p = WORKING_SET_PAGES + SCAN_LEAD_PAGES;
         p < WORKING_SET_PAGES + total_scan_pages(); ++p) {
        auto h = cache.fetch_page(static_cast<uint32_t>(p));
        (void)h;
    }
    if (use_scan_hint_during_scan) {
        set_workload_hint(WorkloadHint::PointLookup);
    }
    for (size_t r = 0; r < REPEAT_WORKING_SET; ++r) {
        for (size_t p = 0; p < WORKING_SET_PAGES; ++p) {
            auto h = cache.fetch_page(static_cast<uint32_t>(p));
            (void)h;
        }
    }
}

int main() {
    std::filesystem::remove(BENCH_PATH);
    MappedFile file(BENCH_PATH);
    file.grow_file(WORKING_SET_PAGES + total_scan_pages() + 1);

    std::cout << "Eviction benchmark: working_set=" << WORKING_SET_PAGES
              << ", scan_lead=" << SCAN_LEAD_PAGES << ", scan_tail=" << SCAN_TAIL_PAGES
              << ", capacity=" << CACHE_CAPACITY << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    AccessObserver observer_lru(256);
    LruPageEvictionPolicy lru_policy;
    {
        PageCache cache(file, CACHE_CAPACITY, &observer_lru, &lru_policy);
        run_workload(cache, observer_lru, false);
        std::cout << "[LRU] hits=" << cache.get_hits() << ", misses=" << cache.get_misses() << std::endl;
    }

    AccessObserver observer_hint(256);
    HintAwarePageEvictionPolicy hint_policy(&observer_hint);
    {
        PageCache cache(file, CACHE_CAPACITY, &observer_hint, &hint_policy);
        run_workload(cache, observer_hint, true);
        std::cout << "[HintAware + Scan hint] hits=" << cache.get_hits() << ", misses=" << cache.get_misses() << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    std::filesystem::remove(BENCH_PATH);
    return 0;
}

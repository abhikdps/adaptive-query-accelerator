#include "storage/page.h"
#include "storage/storage_engine.h"
#include "naive_storage.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <ratio>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <filesystem>

using namespace aqa;

struct BenchmarkConfig {
    std::string db_path = "benchmark_data.db";
    uint32_t total_pages = 10000;
    uint32_t ops_count = 50000;
    size_t  cache_size = 1000;
};

class Timer {
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point start_;

    public:
        Timer() : start_(Clock::now()) {}
        double elapsed_ms() {
            auto end = Clock::now();
            return std::chrono::duration<double, std::milli>(end - start_).count();
        }
};

void bench_naive_random(const BenchmarkConfig& config, const std::vector<uint32_t>& indices) {
    NaiveStorage store(config.db_path);
    RawPage buffer;

    Timer t;
    for (uint32_t idx : indices) {
        store.read_page(idx, buffer);
        volatile uint32_t magic = buffer.header.magic;
        (void)magic;
    }
    double ms = t.elapsed_ms();

    std::cout << " [Naive IO]   Time: " << std::setw(8) << ms << " ms | "
              << "Throughput: " << std::setw(8) << (config.ops_count / (ms/1000.0)) << " ops/s" << std::endl;
}

void bench_optimized_random(const BenchmarkConfig& config, const std::vector<uint32_t>& indices) {
    StorageEngine db(config.db_path, config.cache_size);

    Timer t;
    for (uint32_t idx : indices) {
        auto handle = db.fetch_page(idx);
        volatile uint32_t magic = handle->get_header().magic;
        (void)magic;
    }
    double ms = t.elapsed_ms();

    std::cout << " [PageCache]  Time: " << std::setw(8) << ms << " ms | "
              << "Throughput: " << std::setw(8) << (config.ops_count / (ms/1000.0)) <<" ops/s" << std::endl;

    std::cout << "              Stats: Hits=" << db.get_cache_hits()
              << " Misses=" << db.get_cache_misses()
              << " Ratio=" << (100.0 * db.get_cache_hits() / config.ops_count) << "%" << std::endl;
}

int main() {
    BenchmarkConfig config;
    if (std::filesystem::exists(config.db_path)) {
        std::filesystem::remove(config.db_path);
    }

    std::cout << "Generating " << config.total_pages << " pages for benchmark.." << std::endl;
    {
        StorageEngine setup(config.db_path);
        if (setup.get_total_pages() < config.total_pages) {
            for (uint32_t i = setup.get_total_pages(); i < config.total_pages; ++i) {
                setup.allocate_page();
            }
        }
    }

    std::cout << "Generating workload (" << config.ops_count << " random reads).." << std::endl;
    std::vector<uint32_t> indices;
    indices.reserve(config.ops_count);

    std::mt19937 gen(42);
    std::uniform_int_distribution<uint32_t> hot_dist(0, 500);
    std::uniform_int_distribution<uint32_t> cold_dist(0, config.total_pages - 1);
    std::bernoulli_distribution is_hot(0.8);

    for (size_t i = 0; i < config.ops_count; ++i) {
        if (is_hot(gen)) indices.push_back(hot_dist(gen));
        else indices.push_back(cold_dist(gen));
    }

    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "Running Comparison (Random Read / 80-20 Skew)" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    bench_naive_random(config, indices);
    bench_optimized_random(config, indices);

    std::filesystem::remove(config.db_path);
    return 0;
}

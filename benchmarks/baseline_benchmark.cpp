#include "storage/storage_engine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

using namespace aqa;

struct BenchmarkConfig {
    std::string db_path = "benchmark_data.db";
    uint32_t total_pages = 10000;
    uint32_t read_batch_size = 1000;
    int iterations = 10;
};

void run_sequential_scan(StorageEngine& db, const BenchmarkConfig& config) {
    std::cout << "Running warm up for Sequential_Scan..." << std::endl;
    for (uint32_t i = 0; i < 100; ++i) {
        auto h = db.fetch_page(i % config.total_pages);
        (void)h;
    }

    std::cout << "Running measurements for Sequential_Scan.." << std::endl;

    std::vector<double> latencies;
    auto start_total = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < config.iterations; ++iter) {
        for (uint32_t i = 0; i < config.read_batch_size; ++i) {
            auto handle = db.fetch_page(i % config.total_pages);

            volatile uint32_t magic = handle->get_header().magic;
            (void)magic;
        }
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> total_ms = end_total - start_total;

    double total_reads = config.read_batch_size * config.iterations;
    double avg_latency = (total_ms.count() * 1000) / total_reads;
    double throughput = total_reads / (total_ms.count() / 1000.0);

    std::cout << "Results [Sequential_Scan]:" << std::endl;
    std::cout << "  Avg Total Time: " << total_ms.count() << " ms" << std::endl;
    std::cout << "  Avg Latency:    " << avg_latency << " us/page" << std::endl;
    std::cout << "  Throughput:     " << throughput << " pages/sec" << std::endl << std::endl;
}

void run_random_access(StorageEngine& db, const BenchmarkConfig& config) {
    std::mt19937 gen(42);
    std::uniform_int_distribution<uint32_t> dist(0, config.total_pages - 1);

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < config.read_batch_size * config.iterations; ++i) {
        indices.push_back(dist(gen));
    }

    std::cout << "Running warm up for Random Access..." << std::endl;
    for (uint32_t i = 0; i < 100; ++i) {
        auto h = db.fetch_page(indices[i]);
        (void)h;
    }

    std::cout << "Running measurements for Random Access.." << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    for (uint32_t idx : indices) {
        auto handle = db.fetch_page(idx);
        volatile uint32_t magic = handle->get_header().magic;
        (void)magic;
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> total_ms = end_total - start_total;

    double total_reads = indices.size();
    double avg_latency = (total_ms.count() * 1000) / total_reads;
    double throughput = total_reads / (total_ms.count() / 1000.0);

    std::cout << "Results [Random Access]:" << std::endl;
    std::cout << "  Avg Total Time: " << total_ms.count() << " ms" << std::endl;
    std::cout << "  Avg Latency:    " << avg_latency << " us/page" << std::endl;
    std::cout << "  Throughput:     " << throughput << " pages/sec" << std::endl << std::endl;
}

int main() {
    std::cout << "======= Starting Benchmark Harness =======" << std::endl;

    BenchmarkConfig config;

    aqa::StorageEngine db(config.db_path);
    if (db.get_total_pages() < config.total_pages) {
        std::cout << "Expanding DB to " << config.total_pages << " pages..." << std::endl;
        for (uint32_t i = db.get_total_pages(); i < config.total_pages; ++i) {
            db.allocate_page();
        }
    }

    std::cout << "Configuration:" << std::endl;
    std::cout << "  Pages in DB:  " << db.get_total_pages() << std::endl;
    std::cout << "  Read Batch:   " << config.read_batch_size << std::endl;
    std::cout << "  Iterations:   " << config.iterations << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    run_sequential_scan(db, config);
    run_random_access(db, config);

    return 0;
}

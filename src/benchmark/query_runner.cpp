#include "benchmark/query_runner.h"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <iomanip>

namespace aqa {
    QueryRunner::QueryRunner(StorageEngine& engine) : engine_(engine) {}

    void QueryRunner::run_sequential_scan(uint32_t count) {
        std::cout << "Running sequential scan of " << count << " pages.." << std::endl;

        RawPage p;
        auto start = std::chrono::high_resolution_clock::now();

        for (uint32_t i = 0; i < count; ++i) {
            engine_.read_page(i, p);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        print_stats("Sequential scan", duration.count(), count);
    }

    void QueryRunner::run_random_access(uint32_t count, uint32_t total_pages_in_file) {
        std::cout << "Starting random access of " << count << "pages (Pool size: " << total_pages_in_file << ")" << std::endl;
        std::vector<uint32_t> page_ids(count);
        std::mt19937 gen(42);
        std::uniform_int_distribution<uint32_t> dist(0, total_pages_in_file - 1);

        for (size_t i = 0; i < count; ++i) {
            page_ids[i] = dist(gen);
        }

        RawPage p;
        auto start = std::chrono::high_resolution_clock::now();

        for (uint32_t id : page_ids) {
            engine_.read_page(id, p);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        print_stats("Random Access", duration.count(), count);
    }

    void QueryRunner::print_stats(const std::string& label, double duration_ms, uint32_t count) {
        double avg_latency_us = (duration_ms * 1000.0) / count;
        double throughput_pages_sec = (count / duration_ms) * 1000.0;
        double throughput_mb_sec = (throughput_pages_sec * 4096) / 1024 * 1024;

        std::cout << "---------------------------------------------------------" << std::endl;
        std::cout << "Benchmark:       " << label << std::endl;
        std::cout << "Total time:      " << std::fixed << std::setprecision(3) << duration_ms <<std::endl;
        std::cout << "Average Latency: " << avg_latency_us << " us/page" << std::endl;
        std::cout << "Throughput:      " << throughput_pages_sec << " pages/sec" << std::endl;
        std::cout << "Bandwidth:       " << throughput_mb_sec << " MB/s" << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl;
    }
}

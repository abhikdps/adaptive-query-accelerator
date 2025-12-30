#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <random>

#include "storage/storage_engine.h"
#include "utils/data_generator.h"

using namespace aqa;

struct BenchmarkConfig {
    std::string db_path = "benchmark_data.db";
    uint32_t total_pages = 10000;
    uint32_t read_count = 1000;
    int iterations = 10;
    int warmup_runs = 3;
    std::string log_file = "benchmark_results.log";
};

struct RunResult {
    double duration_ms;
    double throughput_ops_sec;
};

class BenchmarkHarness {
    public:
        explicit BenchmarkHarness(const BenchmarkConfig& config) : config_(config) {}

        void run() {
            setup();

            std::cout << "======= Starting Benchmark Harness =======" << std::endl;
            std::cout << "Configuration:" << std::endl;
            std::cout << "  Pages in DB:  " << config_.total_pages << std::endl;
            std::cout << "  Read Batch:   " << config_.read_count << std::endl;
            std::cout << "  Iterations:   " << config_.iterations << std::endl;
            std::cout << "------------------------------------------" << std::endl;

            run_benchmark("Sequential_Scan", [&](StorageEngine& db) {
                Page p;
                for (uint32_t i = 0; i < config_.read_count; ++i) {
                    db.read_page(i % config_.total_pages, p);
                }
            });

            std::vector<uint32_t> random_indices(config_.read_count);
            std::mt19937 gen(42);
            std::uniform_int_distribution<uint32_t> dist(0, config_.total_pages - 1);
            for(auto& idx : random_indices) idx = dist(gen);

            run_benchmark("Random_Access", [&](StorageEngine& db) {
                Page p;
                for (uint32_t idx : random_indices) {
                    db.read_page(idx, p);
                }
            });

            teardown();
        }

    private:
        BenchmarkConfig config_;

        void setup() {
            std::cout << "Generating " << config_.total_pages << " pages.." << std::endl;
            StorageEngine db(config_.db_path);
            DataGenerator gen(db);
            gen.generate_pages(config_.total_pages);
        }

        void teardown() {
            std::filesystem::remove(config_.db_path);
        }

        template <typename Func>
        void run_benchmark(const std::string& name, Func workload) {
            StorageEngine db(config_.db_path);
            std::vector<RunResult> results;

            std::cout << "Running warm up for " << name << "..." << std::endl;
            for (int i = 0; i < config_.warmup_runs; ++i) {
                workload(db);
            }

            std::cout << "Running measurements for " << name << ".." << std::endl;
            for (int i = 0; i < config_.iterations; ++i) {
                auto start = std::chrono::steady_clock::now();

                workload(db);

                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double, std::milli> diff = end - start;

                double ms = diff.count();
                double tps = (config_.read_count / ms) * 1000.0;
                results.push_back({ms, tps});
            }

            log_results(name, results);
        }

        void log_results(const std::string& name, const std::vector<RunResult>& results) {
            double total_time = 0;
            double min_time = 1e9;
            double max_time = 0;

            for (const auto& r : results) {
                total_time += r.duration_ms;
                if (r.duration_ms < min_time) min_time = r.duration_ms;
                if (r.duration_ms > max_time) max_time = r.duration_ms;
            }

            double avg_time = total_time / results.size();
            double avg_latency = avg_time / config_.read_count * 1000.0;
            double avg_throughput = (config_.read_count / avg_time) * 1000.0;

            std::cout << "Results [" << name << "]:" << std::endl;
            std::cout << "  Avg Total Time: " << std::fixed << std::setprecision(3) << avg_time << " ms" << std::endl;
            std::cout << "  Avg Latency:    " << avg_latency << " us/page" << std::endl;
            std::cout << "  Throughput:     " << avg_throughput << " pages/sec" << std::endl;
            std::cout << std::endl;

            std::ofstream log(config_.log_file, std::ios::app);

            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            log << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << ","
                << name << ","
                << config_.iterations << ","
                << avg_time << ","
                << avg_latency << ","
                << avg_throughput << "\n";
        }
};

int main() {
    BenchmarkConfig config;

    BenchmarkHarness harness(config);
    harness.run();

    return 0;
}

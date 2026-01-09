#include "db/database.h"
#include "storage/page.h"
#include "storage/storage_reader.h"
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <random>
#include <iomanip>

using namespace aqa;

const int NUM_RECORDS = 1000000;
const int READ_SAMPLES = 10000;
const std::string DB_PATH = "storage_bench.db";
const std::string WAL_PATH = DB_PATH + ".wal";

std::vector<uint8_t> make_key(int i) {
    std::string s = "user_id_" + std::to_string(i);
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::vector<uint8_t> make_val(int i) {
    std::string s = "data_payload_" + std::to_string(i);
    s.resize(64, 'x');
    return std::vector<uint8_t>(s.begin(), s.end());
}

template<typename Func>
double measure_sec(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

int main() {
    if (std::filesystem::exists(DB_PATH)) {
        std::filesystem::remove(DB_PATH);
    }
        if (std::filesystem::exists(WAL_PATH)) {
        std::filesystem::remove(WAL_PATH);
    }

    std::cout << "============================================" << std::endl;
    std::cout << "        AQA STORAGE ENGINE BENCHMARK        " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "Records: " << NUM_RECORDS << std::endl;
    std::cout << "Path:    " << DB_PATH << std::endl;
    std::cout << "--------------------------------------------" << std::endl;

    {
        std::cout << "[Write] Inserting " << NUM_RECORDS << " records.." << std::flush;
        Database db(DB_PATH);

        double duration = measure_sec([&]() {
            for(int i = 0; i < NUM_RECORDS; ++i) {
                db.put(make_key(i), make_val(i));
            }
        });

        double ops = NUM_RECORDS / duration;
        std::cout << " Done!" << std::endl;
        std::cout << "  -> Time: " << duration << " s" << std::endl;
        std::cout << "  -> Throughput: " << std::fixed << std::setprecision(2) << ops << " ops/sec" <<std::endl;
    }

    Database db(DB_PATH, 1000, 20000);

    std::vector<int> target_ids;
    target_ids.reserve(READ_SAMPLES);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, NUM_RECORDS - 1);

    for(int i = 0; i < READ_SAMPLES; ++i) {
        target_ids.push_back(dist(rng));
    }

    {
        std::cout << "[Read-Cold] Reading " << READ_SAMPLES << " random keys.." << std::flush;

        double duration = measure_sec([&]() {
            for(int id: target_ids) {
                auto res = db.get(make_key(id));
                if (!res) std::cerr << "Error: Key missing!" << std::endl;
            }
        });

        double latency_us = (duration * 1000000) / READ_SAMPLES;
        std::cout << " Done!" << std::endl;
        std::cout << "  -> Avg Latency: " << latency_us << " us" << std::endl;
    }

    {
        std::cout << "[Read-Warm] Reading same " << READ_SAMPLES << " keys.." << std::flush;

        double duration = measure_sec([&]() {
            for(int id : target_ids) {
                    auto res = db.get(make_key(id));
                    if (!res) std::cerr << "Error: Key missing!" << std::endl;
            }
        });

        double latency_us = (duration * 1000000) / READ_SAMPLES;
        std::cout << " Done!" << std::endl;
        std::cout << "  -> Avg Latency: " << latency_us << " us" << std::endl;
    }

    {
        StorageReader reader(*db.get_engine_ptr_for_benchmarking());

        std::cout << "[Scan] Full table scan (" << NUM_RECORDS << " records)..." << std::flush;

        size_t count = 0;
        double duration = measure_sec([&]() {
            reader.scan([&](RecordID, const std::vector<uint8_t>&, const std::vector<uint8_t>&) {
                count++;
            });
        });

        std::cout << " Done!" << std::endl;
        std::cout << "  -> Scanned: " << count << std::endl;
        std::cout << "  -> Time: " << duration << " s" << std::endl;
        std::cout << "  -> Speed: " << (count / duration) << " records/sec" << std::endl;
    }

    std::filesystem::remove(DB_PATH);
    std::filesystem::remove(WAL_PATH);
    return 0;
}

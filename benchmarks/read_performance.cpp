#include "db/database.h"
#include "storage/page.h"
#include "storage/storage_engine.h"
#include "storage/storage_reader.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>

using namespace aqa;

template<typename Func>
double measure_ms(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::vector<uint8_t> make_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

int main() {
    std::string path = "read_bench.db";
    std::filesystem::remove(path);

    const int NUM_RECORDS = 50000;
    const std::string TARGET_KEY = "key_45000";

    std::cout << "1. Generating " << NUM_RECORDS << " records.." << std::endl;
    {
        Database db(path);
        for(int i = 0; i < NUM_RECORDS; ++i) {
            db.put(make_bytes("key_" + std::to_string(i)),
                   make_bytes("value_data_" + std::to_string(i)));
        }
    }

    Database db(path);
    StorageReader raw_reader(*db.get_engine_ptr_for_benchmarking());

    auto target_bytes = make_bytes(TARGET_KEY);

    std::cout << "2. Running full table scan.." << std::endl;
    bool found_scan = false;

    StorageEngine engine(path, 100);
    StorageReader reader(engine);

    double scan_time = measure_ms([&]() {
        reader.scan([&](RecordID rid, const std::vector<uint8_t>& k, const std::vector<uint8_t>& v) {
            (void)rid;
            (void)v;
            if (!found_scan && k == target_bytes) {
                found_scan = true;
            }
        });
    });

    std::cout << "3. Running Indexed get..." << std::endl;
    bool found_index = false;

    db.get(target_bytes);

    double index_time = measure_ms([&]() {
        auto result = db.get(target_bytes);
        if (result) found_index = true;
    });

    std::cout << "\n-------- Results ---------" << std::endl;
    std::cout << "Scan Time:   " << scan_time << " ms" << std::endl;
    std::cout << "Index Time:  " << index_time << " ms" <<std::endl;
    std::cout << "Speedup:     " << (scan_time / index_time) << "x" << std::endl;

    if (!found_scan || !found_index) {
        std::cerr << "Error: Failed to find key!" << std::endl;
        return 1;
    }

    std::filesystem::remove(path);
    return 0;
}

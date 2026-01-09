#include "db/database.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <cassert>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }

using namespace aqa;

std::vector<uint8_t> make_key(int i) {
    std::string s = "key_" + std::to_string(i);
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::vector<uint8_t> make_val(int i) {
    std::string s = "val_" + std::to_string(i);
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_concurrency() {
    std::string path = "concurrent_test.db";
    std::string wal_path = path + ".wal";
    std::filesystem::remove(path);
    std::filesystem::remove(wal_path);

    Database db(path);
    const int NUM_THREADS = 8;
    const int OPS_PER_THREAD = 1000;

    std::vector<std::thread> threads;
    std::atomic<int> read_errors{0};

    std::cout << "Launching " << NUM_THREADS << " threads.." << std::endl;

    for(int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&, t]() {
            if (t % 2 == 0) {
                for(int i = 0; i < OPS_PER_THREAD; ++i) {
                    int id = (t * OPS_PER_THREAD) + i;
                    db.put(make_key(id), make_val(id));
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                for(int i = 0; i < OPS_PER_THREAD; ++i) {
                    int target_id = ((t - 1) * OPS_PER_THREAD) + i;

                    auto result = db.get(make_key(target_id));

                    if (result) {
                        std::string val_str(result->begin(), result->end());
                        std::string expected = "val_" + std::to_string(target_id);
                        if (val_str != expected) {
                            read_errors++;
                        }
                    }
                }
            }
        });
    }

    for(auto& t : threads) {
        t.join();
    }

    ASSERT_EQ(read_errors.load(), 0);

    size_t expected_count = (NUM_THREADS / 2) * OPS_PER_THREAD;
    ASSERT_EQ(db.get_record_count(), expected_count);

    std::cout << "Expected count (" << expected_count << ") is equal to number of records in DB ("
              << db.get_record_count() << ")" << std::endl;

    std::cout << "[Pass] Multi-threaded read/write stress test" << std::endl;
    std::filesystem::remove(path);
    std::filesystem::remove(wal_path);
}

int main() {
    test_concurrency();
    return 0;
}

#include "db/database.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace aqa;

std::vector<uint8_t> str_to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./crash_recovery_demo [crash|recover]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string db_path = "crash_test.db";
    std::string key = "survivor_key";
    std::string val = "I_SURVIVED_THE_CRASH";

    if (mode == "crash") {
        std::cout << "[Crash Mode] Initializing DB..." << std::endl;
        Database db(db_path);

        std::cout << "[Crash Mode] Writing key: " << key << "..." << std::endl;
        db.put(str_to_bytes(key), str_to_bytes(val));

        std::cout << "[Crash Mode] Simulating process crash now! (std::abort)" << std::endl;
        std::abort();
    }
    else if (mode == "recover") {
        std::cout << "[Recover Mode] Re-opening DB..." << std::endl;
        Database db(db_path);

        auto result = db.get(str_to_bytes(key));

        if (result) {
            std::string recovered_val(result->begin(), result->end());
            std::cout << "\n----------------------------------------" << std::endl;
            std::cout << "Success! Key found." << std::endl;
            std::cout << "Original: " << val << std::endl;
            std::cout << "Recovered: " << recovered_val << std::endl;
            std::cout << "----------------------------------------\n" << std::endl;
        } else {
            std::cerr << "Failure: Key not found after recovery." << std::endl;
            return 1;
        }
    }

    return 0;
}

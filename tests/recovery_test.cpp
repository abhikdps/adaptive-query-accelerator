#include "db/database.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }
#define ASSERT_TRUE(a) if (!(a)) { std::cerr << "FAIL: Expression false" << std::endl; exit(1); }

std::vector<uint8_t> to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::string from_bytes(const std::vector<uint8_t>& b) {
    return std::string(b.begin(), b.end());
}

void test_recovery() {
    std::string db_path = "recovery_test.db";
    std::filesystem::remove(db_path);

    std::cout << "Step 1: Populating database.." << std::endl;
    {
        aqa::Database db(db_path);
        for(int i = 0; i < 1000; ++i) {
            db.put(to_bytes("key_" + std::to_string(i)), to_bytes("val_" + std::to_string(i)));
        }
        ASSERT_EQ(db.get_record_count(), 1000);
    }

    std::cout << "Step 2: Restarting database (simulating recovery)..." << std::endl;
    {
        aqa::Database db(db_path);

        ASSERT_EQ(db.get_record_count(), 1000);
        std::cout << "Recovery Time: " << db.get_recovery_time_ms() << " ms" << std::endl;

        auto val = db.get(to_bytes("key_42"));
        ASSERT_TRUE(val.has_value());
        ASSERT_EQ(from_bytes(*val), "val_42");

        auto val_last = db.get(to_bytes("key_999"));
        ASSERT_TRUE(val_last.has_value());
        ASSERT_EQ(from_bytes(*val_last), "val_999");

        db.put(to_bytes("new_key"), to_bytes("new_val"));
        ASSERT_EQ(db.get_record_count(), 1001);
    }

    std::filesystem::remove(db_path);
    std::cout << "[Pass] Database recovery test" << std::endl;
}

int main() {
    test_recovery();
    return 0;
}

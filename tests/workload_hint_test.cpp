#include "workload_hint.h"
#include "db/database.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b << std::endl; \
        exit(1); \
    }
#define ASSERT_TRUE(c) \
    if (!(c)) { \
        std::cerr << "FAIL: " << #c << std::endl; \
        exit(1); \
    }

static std::vector<uint8_t> to_bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

void test_default_and_set_get() {
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);
    aqa::set_workload_hint(aqa::WorkloadHint::Scan);
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);
}

void test_scan_scope_restores_point_lookup() {
    {
        aqa::ScanScope scope;
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    }
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);
}

void test_scan_scope_restores_previous_hint() {
    aqa::set_workload_hint(aqa::WorkloadHint::Scan);
    {
        aqa::ScanScope scope;
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    }
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
}

void test_nested_scan_scope() {
    aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
    {
        aqa::ScanScope outer;
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
        {
            aqa::ScanScope inner;
            ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
        }
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    }
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);
}

void test_database_scan_sets_hint_during_callback() {
    std::string path = "workload_hint_scan.db";
    std::filesystem::remove(path);
    std::filesystem::remove(path + ".wal");
    {
        aqa::Database db(path);
        db.put(to_bytes("k1"), to_bytes("v1"));
        db.put(to_bytes("k2"), to_bytes("v2"));
    }
    {
        aqa::Database db(path);
        db.scan([&](aqa::RecordID, const std::vector<uint8_t>&, const std::vector<uint8_t>&) {
            ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
        });
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);
    }
    std::filesystem::remove(path);
    std::filesystem::remove(path + ".wal");
}

int main() {
    test_default_and_set_get();
    test_scan_scope_restores_point_lookup();
    test_scan_scope_restores_previous_hint();
    test_nested_scan_scope();
    test_database_scan_sets_hint_during_callback();
    std::cout << "All workload hint tests passed." << std::endl;
    return 0;
}

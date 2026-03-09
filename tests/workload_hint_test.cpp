#include "workload_hint.h"
#include <iostream>

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " != " << #b << std::endl; \
        exit(1); \
    }

int main() {
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);

    aqa::set_workload_hint(aqa::WorkloadHint::Scan);
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);

    aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);

    {
        aqa::ScanScope scope;
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    }
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::PointLookup);

    {
        aqa::set_workload_hint(aqa::WorkloadHint::Scan);
        aqa::ScanScope scope;
        ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);
    }
    ASSERT_EQ(aqa::get_workload_hint(), aqa::WorkloadHint::Scan);

    aqa::set_workload_hint(aqa::WorkloadHint::PointLookup);
    std::cout << "All workload hint tests passed." << std::endl;
    return 0;
}

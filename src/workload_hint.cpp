#include "workload_hint.h"

namespace aqa {

    namespace {
        thread_local WorkloadHint g_workload_hint = WorkloadHint::PointLookup;
    }

    void set_workload_hint(WorkloadHint hint) {
        g_workload_hint = hint;
    }

    WorkloadHint get_workload_hint() {
        return g_workload_hint;
    }

    ScanScope::ScanScope() : previous_(get_workload_hint()) {
        set_workload_hint(WorkloadHint::Scan);
    }

    ScanScope::~ScanScope() {
        set_workload_hint(previous_);
    }

}

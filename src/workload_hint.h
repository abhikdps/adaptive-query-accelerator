#ifndef ADAPTIVE_QUERY_ACCELERATOR_WORKLOAD_HINT_H
#define ADAPTIVE_QUERY_ACCELERATOR_WORKLOAD_HINT_H

namespace aqa {

    enum class WorkloadHint {
        PointLookup,
        Scan
    };

    void set_workload_hint(WorkloadHint hint);
    WorkloadHint get_workload_hint();

    class ScanScope {
        public:
            ScanScope();
            ~ScanScope();
            ScanScope(const ScanScope&) = delete;
            ScanScope& operator=(const ScanScope&) = delete;
        private:
            WorkloadHint previous_;
    };

}

#endif

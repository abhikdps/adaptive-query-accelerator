#ifndef ADAPTIVE_QUERY_ACCELERATOR_QUERY_RUNNER_H
#define ADAPTIVE_QUERY_ACCELERATOR_QUERY_RUNNER_H

#include "storage/storage_engine.h"
#include <cstdint>

namespace aqa {
    class QueryRunner {
        public:
            explicit QueryRunner(StorageEngine& engine);
            void run_sequential_scan(uint32_t count);
            void run_random_access(uint32_t count, uint32_t total_pages_in_file);

        private:
            StorageEngine& engine_;
            void print_stats(const std::string& label, double duration_ms, uint32_t count);
    };
}

#endif

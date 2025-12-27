#ifndef ADAPTIVE_QUERY_ACCELERATOR_DATA_GENERATOR_H
#define ADAPTIVE_QUERY_ACCELERATOR_DATA_GENERATOR_H

#include <cstdint>
#include <cstring>
# include "storage/storage_engine.h"

namespace aqa {
    class DataGenerator {
        public:
            explicit DataGenerator(StorageEngine& engine, uint32_t seed = 42);

            void generate_pages(uint32_t count);

        private:
            StorageEngine& engine_;
            uint32_t seed_;

            void fill_page_payload(uint32_t page_id, Page& page);
    };
}

#endif

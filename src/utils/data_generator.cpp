#include "utils/data_generator.h"
#include <iostream>
#include <random>
#include <cstring>

namespace aqa {
    DataGenerator::DataGenerator(StorageEngine& engine, uint32_t seed)
        : engine_(engine), seed_(seed) {}

    void DataGenerator::generate_pages(uint32_t count) {
        for (uint32_t i = 0; i < count; ++i) {
            if (i >=engine_.get_total_pages()) {
                engine_.allocate_page();
            }

            auto handle = engine_.fetch_page(i);
            handle->reset();
            handle->get_header_mut().page_id = i;

            fill_page_payload(i, *handle);
        }

        std::cout << "Finished generating " << count << " pages." << std::endl;
    }

    void DataGenerator::fill_page_payload(uint32_t page_id, Page& page) {
        auto payload_span = page.get_payload_mut();
        uint32_t* raw_data = reinterpret_cast<uint32_t*>(payload_span.data());
        size_t max_intergers = payload_span.size() / sizeof(uint32_t);

        std::mt19937 gen(seed_ + page_id);
        std::uniform_int_distribution<uint32_t> dist(1, 100000);

        for (size_t k = 0; k < max_intergers; ++k) {
            if (k % 4 == 0) {
                raw_data[k] = page_id;
            } else {
                raw_data[k] = dist(gen);
            }
        }
    }
}

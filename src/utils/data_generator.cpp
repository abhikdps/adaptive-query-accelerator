#include "utils/data_generator.h"
#include <iostream>
#include <random>
#include <cstring>

namespace aqa {
    DataGenerator::DataGenerator(StorageEngine& engine, uint32_t seed)
        : engine_(engine), seed_(seed) {}

    void DataGenerator::generate_pages(uint32_t count) {
        engine_.reset_file();

        for (uint32_t i = 0; i < count; ++i) {
            Page page;
            page.reset();

            page.header.page_id = i;
            page.header.tuple_count = 0;

            fill_page_payload(i, page);

            engine_.write_page(i, page);

            if ((i + 1) % 100 == 0) {
                std::cout << "Generator generated " << (i + 1) << " pages.." << std::endl;
            }
        }

        std::cout << "Finished generating " << count << " pages." << std::endl;
    }

    void DataGenerator::fill_page_payload(uint32_t page_id, Page& page) {
        uint32_t* raw_data = reinterpret_cast<uint32_t*>(page.payload);
        size_t max_intergers = sizeof(page.payload) / sizeof(uint32_t);

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

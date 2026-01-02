#include "storage/storage_engine.h"
#include "utils/data_generator.h"
#include "benchmark/query_runner.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout<< "Usage: ./app [generate <count> | read <page_id>]" << std::endl;
        return 0;
    }

    std::string command = argv[1];
    aqa::StorageEngine db("data.db");

    try {
        if (command == "generate") {
            int count = 10;
            if (argc >= 3) count = std::stoi(argv[2]);

            aqa::DataGenerator gen(db);
            gen.generate_pages(count);
        } else if (command == "read") {
            if (argc < 3) {
                std::cout << "Usage: ./app read <page_id>" << std::endl;
                return 1;
            }
            int page_id = std::stoi(argv[2]);
            try {
                auto handle = db.fetch_page(page_id);

                auto& header = handle->get_header();
                auto payload = handle->get_payload();

                const uint32_t* data = reinterpret_cast<const uint32_t*>(payload.data());

                std::cout << "Page: " << page_id << " read successfully" << std::endl;
                std::cout << "Magic: " << std::hex << header.magic << std::dec << std::endl;
                std::cout << "Payload[0] (Should be " << page_id << "): " << data[0] << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error reading page " << page_id << ": " << e.what() << std::endl;
            }
        } else if (command == "benchmark") {
            if (argc < 4) {
                std::cout << "Usage: ./app benchmark [seq|rnd] <count>" << std::endl;
                return 1;
            }

            std::string mode = argv[2];
            int count = std::stoi(argv[3]);

            aqa::QueryRunner runner(db);
            uint32_t total_pages = db.get_total_pages();

            if (total_pages == 0) {
                std::cerr << "Error: Database is empty. Run 'generate' first" << std::endl;
                return 1;
            }

            if (mode == "seq") {
                runner.run_sequential_scan(std::min((uint32_t)count, total_pages));
            } else if (mode == "rnd") {
                runner.run_random_access(count, total_pages);
            } else {
                std::cerr << "Unknown mode.." << mode << std::endl;
                return 1;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

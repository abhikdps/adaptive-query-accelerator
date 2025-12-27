#include "storage/storage_engine.h"
#include "utils/data_generator.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout<< "Usage: ./app [generate <count> | read <page_id>]" << std::endl;
        return 0;
    }

    std::string command = argv[1];
    aqa::StorageEngine db("data.db");

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
        aqa::Page p;
        if (db.read_page(page_id, p)) {
            uint32_t* data = reinterpret_cast<uint32_t*>(p.payload);
            std::cout << "Page " << page_id << " read successfully" << std::endl;
            std::cout << "Magic: " << std::hex << p.header.magic << std::dec << std::endl;
            std::cout << "Payload[0] (Should be " << page_id << "): " << data[0] << std::endl;
        } else {
            std::cout << "Page " << page_id << " not found" << std::endl;
        }
    }

    return 0;
}

#include "storage/storage_engine.h"
#include <iostream>

int main() {
    aqa::StorageEngine db("data.db");

    for (uint32_t i=0; i < 5; ++i) {
        aqa::Page p;
        p.header.page_id = i;

        std::memset(p.payload, 'A' + i, sizeof(p.payload));

        db.write_page(i, p);
        std::cout << "Written Page " << i << std::endl;
    }

    for (uint32_t i = 0; i < 5; ++i) {
        aqa::Page p_read;

        if (db.read_page(i, p_read)) {
            char expected = 'A' + i;
            if (p_read.payload[0] == expected) {
                std::cout << "Read page " << i << " Verified.." << std::endl;
            }
            else {
                std::cout << "Data mismatch at Page " << i << std::endl;
            }
        }
    }

    return 0;
}

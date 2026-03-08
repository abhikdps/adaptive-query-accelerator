#include "storage/mapped_file.h"
#include <iostream>
#include <filesystem>
#include <cstring>
#include <cassert>

#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "Tests Failed: " << #a << " != " << #b << std::endl; exit(1); }

void test_mapped_io() {
    std::string test_file = "test_mmap.db";
    std::filesystem::remove(test_file);

    {
        aqa::MappedFile file(test_file);

        file.grow_file(2);
        ASSERT_EQ(file.get_page_count(), 2);

        aqa::RawPage* p0 = file.get_page(0);
        p0->header.page_id = 123;
        std::strncpy(reinterpret_cast<char*>(p0->payload), "Hello mmap!", sizeof(p0->payload) - 1);
        p0->payload[sizeof(p0->payload) - 1] = '\0';

        aqa::RawPage* p1 = file.get_page(1);
        p1->header.page_id = 456;
        std::strncpy(reinterpret_cast<char*>(p1->payload), "Second Page", sizeof(p1->payload) - 1);
        p1->payload[sizeof(p1->payload) - 1] = '\0';
    }

    {
        aqa::MappedFile file(test_file);

        ASSERT_EQ(file.get_page_count(), 2);

        aqa::RawPage* p0 = file.get_page(0);
        ASSERT_EQ(p0->header.page_id, 123);
        if (std::strcmp((char*)p0->payload, "Hello mmap!") != 0) {
            std::cerr << "Payload mismatch!" << std::endl;
            exit(1);
        }

        aqa::RawPage* p1 = file.get_page(1);
        ASSERT_EQ(p1->header.page_id, 456);
    }

    std::filesystem::remove(test_file);
    std::cout << "[Pass] MappedFile IO Test" << std::endl;
}

int main() {
    test_mapped_io();
    return 0;
}

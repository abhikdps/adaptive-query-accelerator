#include "storage/page.h"
#include <iostream>
#include <vector>
#include <cassert>

#define ASSERT_TRUE(cond) if (!(cond)) { std::cerr << "FAIL: " << #cond << std::endl; exit(1); }
#define ASSERT_EQ(a, b) if ((a) != (b)) { std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; exit(1); }

void test_page_abstraction() {
    std::vector<uint8_t> raw_memory(aqa::PAGE_SIZE);

    auto* raw_ptr = reinterpret_cast<aqa::RawPage*>(raw_memory.data());

    raw_ptr->header.magic = aqa::PAGE_MAGIC;
    raw_ptr->header.page_id = 101;
    raw_ptr->payload[0] = 0xAA;
    raw_ptr->payload[aqa::Page::PAYLOAD_SIZE - 1] = 0xBB;

    aqa::Page page_view(raw_ptr);

    ASSERT_EQ(page_view.get_id(), 101);
    ASSERT_TRUE(page_view.is_valid());

    auto span = page_view.get_payload();
    ASSERT_EQ(span.size(), aqa::Page::PAYLOAD_SIZE);
    ASSERT_EQ(span[0], 0xAA);
    ASSERT_EQ(span[span.size() - 1], 0xBB);

    page_view.get_header_mut().page_id = 999;
    ASSERT_EQ(raw_ptr->header.page_id, 999);

    auto mut_span = page_view.get_payload_mut();
    mut_span[0] = 0xFF;
    ASSERT_EQ(raw_ptr->payload[0], 0xFF);

    std::cout << "[Pass] Page Abstraction Test" << std::endl;
}

int main() {
    test_page_abstraction();
    return 0;
}

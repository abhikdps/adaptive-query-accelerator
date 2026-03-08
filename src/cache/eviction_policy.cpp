#include "cache/eviction_policy.h"

namespace aqa {

    uint32_t LruPageEvictionPolicy::choose_victim(
        const std::vector<uint32_t>& unpinned_page_ids_lru_order) {
        return unpinned_page_ids_lru_order.empty() ? 0 : unpinned_page_ids_lru_order.front();
    }

    size_t LruRecordEvictionPolicy::choose_victim(
        const std::vector<const std::vector<uint8_t>*>& keys_lru_order) {
        (void)keys_lru_order;
        return 0;
    }

}

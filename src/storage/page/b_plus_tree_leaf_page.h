#ifndef AQA_STORAGE_B_PLUS_TREE_LEAF_PAGE_H
#define AQA_STORAGE_B_PLUS_TREE_LEAF_PAGE_H

#include <utility>
#include "storage/page/b_plus_tree_page.h"

namespace aqa {
    #define LEAF_PAGE_HEADER_SIZE 28

    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPlusTreeLeafPage : public BPlusTreePage {
        public:
            void Init(page_id_t page_id, page_id_t parent_id = -1, int max_size = 0);

            page_id_t GetNextPageId() const;
            void SetNextPageId(page_id_t next_page_id);

            KeyType KeyAt(int index) const;
            ValueType ValueAt(int index) const;
            const std::pair<KeyType, ValueType> &GetItem(int index);

            int KeyIndex(const KeyType &key, const KeyComparator &comparator) const;
            int Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);

        private:
            page_id_t next_page_id_;
            std::pair<KeyType, ValueType> array_[1];
    };
}

#endif

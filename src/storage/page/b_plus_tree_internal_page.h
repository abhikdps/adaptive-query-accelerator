#ifndef AQA_STORAGE_B_PLUS_TREE_INTERNAL_PAGE_H
#define AQA_STORAGE_B_PLUS_TREE_INTERNAL_PAGE_H

#include <utility>
#include "storage/page/b_plus_tree_page.h"

namespace aqa {

    #define INTERNAL_PAGE_HEADER_SIZE 24

    template <typename KeyType, typename ValueType, typename KeyComparator>
    class BPlusTreeInternalPage : public BPlusTreePage {
        public:
            void Init(page_id_t page_id, page_id_t parent_id = -1, int max_size = 0);

            ValueType Lookup(const KeyType &key, const KeyComparator &comparator) const;

            void setKeyAt(int index, const KeyType &key);
            KeyType KeyAt(int index) const;
            void setValueAt(int index, const ValueType &value);
            ValueType ValueAt(int index) const;

            int ValueIndex(const ValueType &value) const;
            const std::pair<KeyType, ValueType> &GetItem(int index);

        private:
            std::pair<KeyType, ValueType> array_[1];
    };
}

#endif

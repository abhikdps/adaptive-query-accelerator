#include "storage/page/b_plus_tree_internal_page.h"
#include <algorithm>
#include <functional>

namespace aqa {
    template <typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
        setPageType(IndexPageType::INTERNAL_PAGE);
        setSize(0);
        setPageId(page_id);
        setParentPageId(parent_id);

        if (max_size == 0) {
            int calculated_max = (4096 - INTERNAL_PAGE_HEADER_SIZE) / sizeof(std::pair<KeyType, ValueType>);
            setMaxSize(calculated_max);
        } else {
            setMaxSize(max_size);
        }
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    KeyType BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::KeyAt(int index) const {
        return array_[index].first;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::setKeyAt(int index, const KeyType &key) {
        array_[index].first = key;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::ValueAt(int index) const {
        return array_[index].second;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::setValueAt(int index, const ValueType &value) {
        array_[index].second = value;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    int BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::ValueIndex(const ValueType &value) const {
        for (int i = 0; i < getSize(); ++i) {
            if (array_[i].second == value) {
                return i;
            }
        }
        return -1;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    const std::pair<KeyType, ValueType> &BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::GetItem(int index) {
        return array_[index];
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>::Lookup(const KeyType &key, const KeyComparator &comparator) const {
        if (getSize() == 0) {
            return ValueType{};
        }
        auto it = std::upper_bound(array_ + 1, array_ + getSize(), key,
            [&comparator](const KeyType& k, const std::pair<KeyType, ValueType>& pair) {
                return comparator(k, pair.first);
            }
        );

        int index = std::distance(array_, it);

        return array_[index - 1].second;
    }

    template class BPlusTreeInternalPage<int, int, std::less<int>>;

}

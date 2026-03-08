#include "storage/page/b_plus_tree_leaf_page.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>
#include "storage/page/b_plus_tree_page.h"

namespace aqa {
    template <typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
        setPageType(IndexPageType::LEAF_PAGE);
        setSize(0);
        setPageId(page_id);
        setParentPageId(parent_id);
        SetNextPageId(-1);

        if (max_size == 0) {
            int calculated_max = (4096 - LEAF_PAGE_HEADER_SIZE) / sizeof(std::pair<KeyType, ValueType>);
            setMaxSize(calculated_max);
        } else {
            setMaxSize(max_size);
        }
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    page_id_t BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::GetNextPageId() const {
        return next_page_id_;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    void BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::SetNextPageId(page_id_t next_page_id) {
        next_page_id_ = next_page_id;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    int BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::KeyIndex(const KeyType &key, const KeyComparator &comparator) const {
        auto it = std::lower_bound(array_, array_ + getSize(), key,
            [&comparator](const std::pair<KeyType, ValueType>& pair, const KeyType& k) {
                return comparator(pair.first, k);
            }
        );

        return std::distance(array_, it);
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    KeyType BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::KeyAt(int index) const {
        return array_[index].first;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    ValueType BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::ValueAt(int index) const {
        return array_[index].second;
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    const std::pair<KeyType, ValueType> &BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::GetItem(int index) {
        return array_[index];
    }

    template <typename KeyType, typename ValueType, typename KeyComparator>
    int BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>::Insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator) {
        int current_size = getSize();
        if (current_size >= getMaxSize()) {
            return -1;
        }

        int index = KeyIndex(key, comparator);
        if (index < current_size) {
            KeyType existing_key = KeyAt(index);
            if (!comparator(existing_key, key) && !comparator(key, existing_key)) {
                return -1;
            }
        }

        for (int i = current_size; i > index; i--) {
            array_[i] = array_[i - 1];
        }

        array_[index] = {key, value};

        increaseSize(1);

        return current_size + 1;
    }

    template class BPlusTreeLeafPage<int, int, std::less<int>>;
}

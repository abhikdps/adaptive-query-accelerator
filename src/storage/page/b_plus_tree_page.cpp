#include "storage/page/b_plus_tree_page.h"

namespace aqa {
    bool BPlusTreePage::IsLeafPage() const {
        return page_type_ == IndexPageType::LEAF_PAGE;
    }

    bool BPlusTreePage::IsRootPage() const {
        return parent_page_id_ == -1;
    }

    void BPlusTreePage::setPageType(IndexPageType page_type) {
        page_type_ = page_type;
    }

    int BPlusTreePage::getSize() const {
        return size_;
    }

    void BPlusTreePage::setSize(int size) {
        size_ = size;
    }

    void BPlusTreePage::increaseSize(int amount) {
        size_ += amount;
    }

    int BPlusTreePage::getMaxSize() const {
        return max_size_;
    }

    void BPlusTreePage::setMaxSize(int max_size) {
        max_size_ = max_size;
    }

    int BPlusTreePage::getMinSize() const {
        if (IsRootPage()) {
            return IsLeafPage() ? 1 : 2;
        }
        return (max_size_) / 2;
    }

    page_id_t BPlusTreePage::getParentPageId() const {
        return parent_page_id_;
    }

    void BPlusTreePage::setParentPageId(page_id_t parent_page_id) {
        parent_page_id_ = parent_page_id;
    }

    lsn_t BPlusTreePage::getLSN() const {
        return lsn_;
    }

    void BPlusTreePage::setLSN(lsn_t lsn) {
        lsn_ = lsn;
    }

    page_id_t BPlusTreePage::getPageId() const {
    return page_id_;
}

    void BPlusTreePage::setPageId(page_id_t page_id) {
        page_id_ = page_id;
    }
}

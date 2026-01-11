#ifndef AQA_STORAGE_B_PLUS_TREE_PAGE_H
#define AQA_STORAGE_B_PLUS_TREE_PAGE_H

#include <cstring>
#include <cassert>

namespace aqa {
    using page_id_t = int;
    using lsn_t = int;

    enum class IndexPageType { INVALID_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE};

    class BPlusTreePage {
        public:
            bool IsLeafPage() const;
            bool IsRootPage() const;
            void setPageType(IndexPageType page_type);

            int getSize() const;
            void setSize(int size);
            void increaseSize(int amount);

            int getMaxSize() const;
            void setMaxSize(int max_size);

            int getMinSize() const;

            page_id_t getParentPageId() const;
            void setParentPageId(page_id_t parent_page_id);

            page_id_t getPageId() const;
            void setPageId(page_id_t page_id);

            lsn_t getLSN() const;
            void setLSN(lsn_t lsn);

        protected:
            IndexPageType page_type_;
            lsn_t lsn_;
            int size_;
            int max_size_;
            page_id_t parent_page_id_;
            page_id_t page_id_;
    };
}

#endif

#ifndef ADAPTIVE_QUERY_ACCELERATOR_PAGE_HANDLE_H
#define ADAPTIVE_QUERY_ACCELERATOR_PAGE_HANDLE_H

#include "storage/page.h"

namespace aqa {
    class PageCache;

    class PageHandle {
        public:
            PageHandle() : cache_(nullptr), page_(nullptr), page_id_(0) {}

            PageHandle(PageCache* cache, Page page, uint32_t page_id);

            ~PageHandle();

            PageHandle(const PageHandle&) = delete;
            PageHandle& operator=(const PageHandle&) = delete;

            PageHandle(PageHandle&& other) noexcept;
            PageHandle& operator=(PageHandle&& other) noexcept;

            Page* operator->() { return &page_; }
            const Page* operator->() const { return &page_; }

            Page& operator*() { return page_; }
            const Page& operator*() const { return page_; }

            [[nodiscard]] bool is_valid() const { return cache_ != nullptr; }

        private:
            PageCache* cache_;
            Page page_;
            uint32_t page_id_;
    };
}

#endif

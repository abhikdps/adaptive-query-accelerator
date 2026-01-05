#ifndef NAIVE_STORAGE_H
#define NAIVE_STORAGE_H

#include <cstdint>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include "storage/page.h"

namespace aqa {
    class NaiveStorage {
        public:
            NaiveStorage(const std::string& path) {
                fd_ = open(path.c_str(), O_RDWR | O_CREAT, 0644);
                if (fd_ == -1) throw std::runtime_error("Failed to open file");
            }

            ~NaiveStorage() {
                if (fd_ != -1) close(fd_);
            }

            void read_page(uint32_t page_id, RawPage& out_page) {
                off_t offset = static_cast<off_t>(page_id) * PAGE_SIZE;
                ssize_t bytes = pread(fd_, &out_page, PAGE_SIZE, offset);
                if (bytes != PAGE_SIZE) {
                    std::memset(reinterpret_cast<void*>(&out_page), 0, PAGE_SIZE);
                }
            }

            void write_page(uint32_t page_id, const RawPage& in_page) {
                off_t offset = static_cast<off_t>(page_id) * PAGE_SIZE;
                pwrite(fd_, &in_page, PAGE_SIZE, offset);
            }

        private:
            int fd_;
    };
}

#endif

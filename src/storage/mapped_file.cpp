#include "storage/mapped_file.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

namespace aqa {
    MappedFile::MappedFile(const std::string& path) : path_(path), fd_(-1), data_(nullptr), file_size_(0), page_count_(0) {
        fd_ = open(path_.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ == -1) {
            throw std::runtime_error("Failed to oprn file: " + path_);
        }

        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            close(fd_);
            throw std::runtime_error("Failed to stat file: " + path_);
        }
        file_size_ = static_cast<size_t>(sb.st_size);

        if (file_size_ == 0) {
            grow_file(1);
            return;
        }

        page_count_ = file_size_ / PAGE_SIZE;

        map_memory();
    }

    MappedFile::~MappedFile() {
        unmap_memory();
        if (fd_ != -1) {
            close(fd_);
        }
    }

    void MappedFile::map_memory() {
        data_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);

        if (data_ == MAP_FAILED) {
            data_ = nullptr;
            close(fd_);
            fd_ = -1;
            throw std::runtime_error("mmap failed for file: " + path_);
        }
    }

    void MappedFile::unmap_memory() {
        if (data_) {
            msync(data_, file_size_, MS_SYNC);
            munmap(data_, file_size_);
            data_ = nullptr;
        }
    }

    Page* MappedFile::get_page(uint32_t page_id) {
        uint8_t* byte_ptr= static_cast<uint8_t*>(data_);
        return reinterpret_cast<Page*>(byte_ptr + (page_id * PAGE_SIZE));
    }

    void MappedFile::grow_file(uint32_t new_page_count) {
        if (new_page_count <= page_count_) return;

        size_t new_size = new_page_count * PAGE_SIZE;

        unmap_memory();

        if (ftruncate(fd_, new_size) == -1) {
            throw std::runtime_error("Failed to resize file: " + path_);
        }

        file_size_ = new_size;
        page_count_ = new_page_count;

        map_memory();
    }

    void MappedFile::flush() {
        if (data_) {
            if (msync(data_, file_size_, MS_SYNC) == -1) {
                throw std::runtime_error("msync failed");
            }
        }
    }
}

#include "storage/mapped_file.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include "storage/page.h"

namespace aqa {
    MappedFile::MappedFile(const std::string& path)
        : path_(path), fd_(-1), data_(nullptr), file_size_(0), page_count_(0) {

        fd_ = open(path_.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ == -1) {
            throw std::runtime_error("Failed to open file: " + path_);
        }

        struct stat sb;
        if (fstat(fd_, &sb) == -1) {
            close(fd_);
            throw std::runtime_error("Failed to stat file: " + path_);
        }
        file_size_ = static_cast<size_t>(sb.st_size);
        page_count_ = file_size_ / PAGE_SIZE;

        if (file_size_ > 0) {
            map_memory();
        }
    }

    MappedFile::~MappedFile() {
        unmap_memory();
        if (fd_ != -1) {
            close(fd_);
        }
    }

    MappedFile::MappedFile(MappedFile&& other) noexcept
        : path_(std::move(other.path_)),
          fd_(other.fd_),
          data_(other.data_),
          file_size_(other.file_size_),
          page_count_(other.page_count_) {

            other.fd_ = -1;
            other.data_ = nullptr;
            other.file_size_ = 0;
            other.page_count_ = 0;
          }

    MappedFile& MappedFile::operator=(MappedFile&& other) noexcept {
        if (this != &other) {
            unmap_memory();
            if (fd_ == -1) close(fd_);

            path_ = std::move(other.path_);
            fd_ = other.fd_;
            data_ = other.data_;
            file_size_ = other.file_size_;
            page_count_ = other.page_count_;

            other.fd_ = -1;
            other.data_ = nullptr;
            other.file_size_ = 0;
            other.page_count_ = 0;
        }
        return *this;
    }

    void MappedFile::map_memory() {
        if (file_size_ == 0) return;

        data_ = mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);

        if (data_ == MAP_FAILED) {
            data_ = nullptr;
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

    RawPage* MappedFile::get_page(uint32_t page_id) {
        uint8_t* byte_ptr = static_cast<uint8_t*>(data_);
        return reinterpret_cast<RawPage*>(byte_ptr + (page_id * PAGE_SIZE));
    }

    void MappedFile::grow_file(uint32_t new_page_count) {
        if (new_page_count <= page_count_) return;

        size_t new_size = new_page_count * PAGE_SIZE;

        if (data_) {
            unmap_memory();
        }

        if (ftruncate(fd_, new_size) == -1) {
            throw std::runtime_error("Failed to resize file: " + path_);
        }
        fsync(fd_);

        file_size_ = new_size;
        page_count_ = new_page_count;

        map_memory();
    }

    void MappedFile::flush() {
        if (data_) {
            if (msync(data_, file_size_, MS_SYNC) == -1) {
                throw std::runtime_error("msync failed");
            }
            fsync(fd_);
        }
    }
}

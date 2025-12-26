#include "storage/storage_engine.h"
#include <iostream>

namespace aqa {
    StorageEngine::StorageEngine(const std::string& file_path)
        : file_path_(file_path) {
            open_file_();
        }

    StorageEngine::~StorageEngine() {
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    }

    void StorageEngine::open_file_() {
        file_stream_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);

        if (!file_stream_.is_open()) {
            std::fstream create_stream(file_path_, std::ios::out | std::ios::binary);
            create_stream.close();

            file_stream_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
        }

        if (!file_stream_.is_open()) {
            throw std::runtime_error("Failed to open database file: " + file_path_);
        }
    }

    void StorageEngine::write_page(uint32_t page_id, const Page& page) {
        file_stream_.clear();

        std::streampos offset = static_cast<std::streampos>(page_id) * PAGE_SIZE;

        file_stream_.seekp(offset, std::ios::beg);
        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to seek for write at page_id: " + std::to_string(page_id));
        }

        file_stream_.write(reinterpret_cast<const char*>(&page), PAGE_SIZE);

        file_stream_.flush();

        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to write page_id: " + std::to_string(page_id));
        }
    }

    bool StorageEngine::read_page(uint32_t page_id, Page& out_page) {
        file_stream_.clear();

        std::streampos offset = static_cast<std::streampos>(page_id) * PAGE_SIZE;

        file_stream_.seekg(0, std::ios::end);
        if (offset >= file_stream_.tellg()) {
            return false;
        }

        file_stream_.seekg(offset, std::ios::beg);
        file_stream_.read(reinterpret_cast<char*>(&out_page), PAGE_SIZE);

        if (file_stream_.fail() && !file_stream_.eof()) {
            return false;
        }

        return true;
    }

    uint32_t StorageEngine::get_total_pages() {
        file_stream_.clear();
        file_stream_.seekg(0, std::ios::end);
        std::streampos file_size = file_stream_.tellg();

        if (file_size < 0) return 0;
        return static_cast<uint32_t>(file_size / PAGE_SIZE);
    }

    void StorageEngine::reset_file() {
        file_stream_.close();

        std::fstream trunc_stream(file_path_, std::ios::out | std::ios::binary | std::ios::trunc);
        trunc_stream.close();

        open_file_();
    }
}

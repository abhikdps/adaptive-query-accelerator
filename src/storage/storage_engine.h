#ifndef ADAPTIVE_QUERY_ACCELERATOR_STORAGE_ENGINE_H
#define ADAPTIVE_QUERY_ACCELERATOR_STORAGE_ENGINE_H

#include <fstream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include "storage/page.h"

namespace aqa {
    class StorageEngine {
        public:
            // constructor
            explicit StorageEngine(const std::string& file_path);

            //descructor
            ~StorageEngine();

            void write_page(uint32_t page_id, const Page& page);
            bool read_page(uint32_t page_id, Page& out_page);
            uint32_t get_total_pages();
            void reset_file();

        private:
            std::string file_path_;
            std::fstream file_stream_;

            void open_file_();
    };
}

#endif

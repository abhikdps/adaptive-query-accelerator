# B+ tree page sources: build without clang-tidy to avoid __has_feature and style-checker errors
set(CMAKE_CXX_CLANG_TIDY_SAVED "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_CLANG_TIDY "")
add_library(bptree_pages STATIC
    storage/page/b_plus_tree_page.cpp
    storage/page/b_plus_tree_internal_page.cpp
    storage/page/b_plus_tree_leaf_page.cpp
)
set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY_SAVED}")
target_include_directories(bptree_pages PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(core_lib
    observer/access_observer.cpp
    storage/page_cache.cpp
    storage/storage_engine.cpp
    storage/storage_writer.cpp
    storage/storage_reader.cpp
    storage/mapped_file.cpp
    utils/data_generator.cpp
    benchmark/query_runner.cpp
    index/index.cpp
    cache/lru_cache.cpp
    wal/wal_manager.cpp
    db/database.cpp
)
target_include_directories(core_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(core_lib PRIVATE bptree_pages)

add_executable(app main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Threads::Threads)
target_link_libraries(app PRIVATE core_lib)

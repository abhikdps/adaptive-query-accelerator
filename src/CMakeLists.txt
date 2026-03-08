add_library(core_lib
    observer/access_observer.cpp
    storage/page_cache.cpp
    storage/storage_engine.cpp
    storage/storage_writer.cpp
    storage/storage_reader.cpp
    storage/mapped_file.cpp
    storage/page/b_plus_tree_page.cpp
    storage/page/b_plus_tree_internal_page.cpp
    storage/page/b_plus_tree_leaf_page.cpp
    utils/data_generator.cpp
    benchmark/query_runner.cpp
    index/index.cpp
    cache/lru_cache.cpp
    wal/wal_manager.cpp
    db/database.cpp
)
target_include_directories(core_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(app main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Threads::Threads)
target_link_libraries(app PRIVATE core_lib)

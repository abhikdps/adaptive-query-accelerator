add_library(core_lib
    lib.cpp
    storage/page_cache.cpp
    storage/storage_engine.cpp
    storage/mapped_file.cpp
    utils/data_generator.cpp
    benchmark/query_runner.cpp
)
target_include_directories(core_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(app main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Threads::Threads)
target_link_libraries(app PRIVATE core_lib)

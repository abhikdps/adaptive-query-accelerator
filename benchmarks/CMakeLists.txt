add_executable(baseline_benchmark baseline_benchmark.cpp)
target_link_libraries(baseline_benchmark PRIVATE core_lib)
target_include_directories(baseline_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/benchmarks)

add_executable(read_performance read_performance.cpp)
target_link_libraries(read_performance PRIVATE core_lib)
target_include_directories(read_performance PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(storage_benchmark storage_benchmark.cpp)
target_link_libraries(storage_benchmark PRIVATE core_lib)
target_include_directories(storage_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(eviction_benchmark eviction_benchmark.cpp)
target_link_libraries(eviction_benchmark PRIVATE core_lib)
target_include_directories(eviction_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)

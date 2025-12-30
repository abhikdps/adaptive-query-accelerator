add_executable(baseline_benchmark baseline_benchmark.cpp)

target_link_libraries(baseline_benchmark PRIVATE core_lib)
target_include_directories(baseline_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(unit_tests mapped_file_test.cpp)

target_link_libraries(unit_tests PRIVATE core_lib)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME MappedFileTest COMMAND unit_tests)
add_test(NAME PageUnitTest COMMAND unit_tests)
add_test(NAME PageCacheTest COMMAND unit_tests)

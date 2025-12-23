include(GoogleTest)

add_executable(unit_tests test_main.cpp)

target_link_libraries(unit_tests GTest::gtest_main core_lib)

gtest_discover_tests(unit_tests)

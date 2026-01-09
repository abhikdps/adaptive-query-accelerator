add_executable(mapped_file_test mapped_file_test.cpp)
target_link_libraries(mapped_file_test PRIVATE core_lib)
target_include_directories(mapped_file_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME MappedFileTest COMMAND mapped_file_test)

add_executable(page_test page_test.cpp)
target_link_libraries(page_test PRIVATE core_lib)
target_include_directories(page_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME PageUnitTest COMMAND page_test)

add_executable(page_cache_test page_cache_test.cpp)
target_link_libraries(page_cache_test PRIVATE core_lib)
target_include_directories(page_cache_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME PageCacheTest COMMAND page_cache_test)

add_executable(pinning_test pinning_test.cpp)
target_link_libraries(pinning_test PRIVATE core_lib)
target_include_directories(pinning_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME PinningTest COMMAND pinning_test)

add_executable(writer_test writer_test.cpp)
target_link_libraries(writer_test PRIVATE core_lib)
target_include_directories(writer_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME WriterTest COMMAND writer_test)

add_executable(reader_test reader_test.cpp)
target_link_libraries(reader_test PRIVATE core_lib)
target_include_directories(reader_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME ReaderTest COMMAND reader_test)

add_executable(integration_test integration_test.cpp)
target_link_libraries(integration_test PRIVATE core_lib)
target_include_directories(integration_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME IntegrationTest COMMAND integration_test)

add_executable(index_test index_test.cpp)
target_link_libraries(index_test PRIVATE core_lib)
target_include_directories(index_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME IndexTest COMMAND index_test)

add_executable(recovery_test recovery_test.cpp)
target_link_libraries(recovery_test PRIVATE core_lib)
target_include_directories(recovery_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME RecoveryTest COMMAND recovery_test)

add_executable(concurrency_test concurrency_test.cpp)
target_link_libraries(concurrency_test PRIVATE core_lib pthread)
target_include_directories(concurrency_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME ConcurrencyTest COMMAND concurrency_test)

add_executable(wal_test wal_test.cpp)
target_link_libraries(wal_test PRIVATE core_lib)
target_include_directories(wal_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME WalTest COMMAND wal_test)

add_executable(crash_recovery_demo crash_recovery_demo.cpp)
target_link_libraries(crash_recovery_demo PRIVATE core_lib)
target_include_directories(crash_recovery_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)

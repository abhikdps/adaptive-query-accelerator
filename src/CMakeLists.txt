add_library(core_lib lib.cpp)
target_include_directories(core_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(app main.cpp)
target_link_libraries(app PRIVATE core_lib)
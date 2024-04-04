project(tests)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(always_passes "${CMAKE_SOURCE_DIR}/tests/always_passes/main.cpp")
target_link_libraries(always_passes PUBLIC gtest_main)

include(GoogleTest)
gtest_discover_tests(always_passes)
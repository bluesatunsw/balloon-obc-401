project(tests)

add_executable(common_tests
    mock/bus.cpp
)
target_link_libraries(common_tests PUBLIC common gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(common_tests)

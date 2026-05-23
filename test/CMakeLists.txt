find_package(GTest REQUIRED)

add_executable(test_main test.cpp)
target_link_libraries(test_main PRIVATE coro GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_main)

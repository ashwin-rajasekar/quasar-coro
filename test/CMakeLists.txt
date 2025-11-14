find_package(GTest REQUIRED)

add_executable(test_main test.cpp)
target_link_libraries(test_main PRIVATE coro GTest::gtest_main)
add_test(NAME test_main COMMAND test_main 12)

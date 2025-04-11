add_executable(DITest DITest.cpp)
add_test(NAME DITest COMMAND DITest)
target_link_libraries(DITest PUBLIC Platform gtest gtest_main)

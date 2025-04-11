add_executable(LoggerTest LoggerTest.cpp)
add_test(NAME LoggerTest COMMAND LoggerTest)
target_link_libraries(LoggerTest PUBLIC Platform gtest gtest_main)


add_executable(ConfigureTest ConfigureTest.cpp)
add_test(NAME ConfigureTest COMMAND ConfigureTest)
target_link_libraries(ConfigureTest PUBLIC Platform gtest gtest_main)

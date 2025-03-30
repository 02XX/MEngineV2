add_executable(loggerTest loggerTest.cpp)
add_test(NAME loggerTest COMMAND LoggerTest)
target_link_libraries(loggerTest PUBLIC Platform gtest gtest_main)


add_executable(sharedPtrTest SharedPtrTest.cpp)
add_test(NAME sharedPtrTest COMMAND SharedPtrTest)
target_link_libraries(sharedPtrTest PUBLIC Platform gtest gtest_main SDL3::SDL3)

# add_executable(bufferTest BufferTest.cpp)
# add_test(NAME bufferTest COMMAND BufferTest)
# target_link_libraries(bufferTest PUBLIC Platform gtest gtest_main)
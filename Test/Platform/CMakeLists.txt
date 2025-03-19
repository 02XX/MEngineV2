add_executable(loggerTest loggerTest.cpp)
add_test(NAME loggerTest COMMAND LoggerTest)
target_link_libraries(loggerTest PUBLIC Platform gtest gtest_main)

# add_executable(commandBufferTest CommandBufferTest.cpp)
# add_test(NAME commandBufferTest COMMAND CommandBufferTest)
# target_link_libraries(commandBufferTest PUBLIC Platform gtest gtest_main)

# add_executable(bufferTest BufferTest.cpp)
# add_test(NAME bufferTest COMMAND BufferTest)
# target_link_libraries(bufferTest PUBLIC Platform gtest gtest_main)
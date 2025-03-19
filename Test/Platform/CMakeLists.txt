add_executable(loggerTest loggerTest.cpp)
add_test(NAME loggerTest COMMAND LoggerTest)
target_link_libraries(loggerTest PUBLIC Platform gtest gtest_main)

add_executable(sharedPipelineTest SharedPipelineTest.cpp)
add_test(NAME sharedPipelineTest COMMAND SharedPipelineTest)
target_link_libraries(sharedPipelineTest PUBLIC Platform gtest gtest_main SDL3::SDL3)

add_executable(sharedPipelineLayoutTest SharedPipelineLayoutTest.cpp)
add_test(NAME sharedPipelineLayoutTest COMMAND SharedPipelineLayoutTest)
target_link_libraries(sharedPipelineLayoutTest PUBLIC Platform gtest gtest_main SDL3::SDL3)

# add_executable(bufferTest BufferTest.cpp)
# add_test(NAME bufferTest COMMAND BufferTest)
# target_link_libraries(bufferTest PUBLIC Platform gtest gtest_main)
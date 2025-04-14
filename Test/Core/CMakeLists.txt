add_executable(TaskSchedulerTest TaskSchedulerTest.cpp)
add_test(NAME TaskSchedulerTest COMMAND TaskSchedulerTest)
target_link_libraries(TaskSchedulerTest PUBLIC Core gtest gtest_main)


add_executable(MemoryAlign MemoryAlign.cpp)
add_test(NAME MemoryAlign COMMAND MemoryAlign)
target_link_libraries(MemoryAlign PUBLIC Core gtest gtest_main)

add_executable(TaskSchedulerTest TaskSchedulerTest.cpp)
add_test(NAME TaskSchedulerTest COMMAND TaskSchedulerTest)
target_link_libraries(TaskSchedulerTest PUBLIC Core gtest gtest_main)


add_executable(MemoryAlign MemoryAlign.cpp)
add_test(NAME MemoryAlign COMMAND MemoryAlign)
target_link_libraries(MemoryAlign PUBLIC Core gtest gtest_main)


add_executable(JsonTest JsonTest.cpp)
add_test(NAME JsonTest COMMAND JsonTest)
target_link_libraries(JsonTest PUBLIC nlohmann_json::nlohmann_json gtest gtest_main)

add_executable(UUIDTest UUIDTest.cpp)
add_test(NAME UUIDTest COMMAND UUIDTest)
target_link_libraries(UUIDTest PUBLIC Core gtest gtest_main)

add_executable(RingBufferTest RingBufferTest.cpp)
add_test(NAME RingBufferTest COMMAND RingBufferTest)
target_link_libraries(RingBufferTest PUBLIC Core gtest gtest_main)
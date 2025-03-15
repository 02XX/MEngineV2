file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS *.cpp)
add_executable(loggerTest ${SOURCES})

add_test(NAME loggerTest COMMAND loggerTest)
target_link_libraries(loggerTest PUBLIC Platform gtest gtest_main)
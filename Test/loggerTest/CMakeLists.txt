
add_executable(loggerTest main.cpp)

add_test(NAME loggerTest COMMAND loggerTest)

target_link_libraries(loggerTest PUBLIC Platform)
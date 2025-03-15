cmake_minimum_required(VERSION 3.20.0)
project(MEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件的输出目录 (bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 设置动态库（共享库）的输出目录 (lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置静态库的输出目录 (lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 启用位置无关编码 -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32)
    message("Platform: Windows")
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX)
    message("Platform: Unix")
    if(APPLE)
        message("Platform: MacOS")
        add_definitions(-DPLATFORM_MACOS)
    else()
        message("Platform: Linux")
        add_definitions(-DPLATFORM_LINUX)
    endif()
else()
    message("Platform: Unknown")
endif()

add_subdirectory(Tool)
add_subdirectory(Function)
add_subdirectory(Resource)
add_subdirectory(Core)
add_subdirectory(Platform)

#3rd party
add_subdirectory(3rdParty/spdlog)
add_subdirectory(3rdParty/googletest)


# test
enable_testing()
add_subdirectory(Test)


file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Platform SHARED ${SOURCES})

find_package(spdlog CONFIG REQUIRED)


target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Platform
    PUBLIC
    spdlog::spdlog
)

target_compile_definitions(Platform PRIVATE MENGINE_EXPORT)

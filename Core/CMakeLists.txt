
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Core SHARED ${SOURCES})



target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Core
    PUBLIC
    Platform
)
target_compile_definitions(Core PRIVATE MENGINE_EXPORT)

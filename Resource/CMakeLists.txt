
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Resource STATIC ${SOURCES})



target_include_directories(Resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Resource
    PUBLIC
    Core
    Platform
    EnTT::EnTT
)
# target_compile_definitions(Resource PRIVATE MENGINE_EXPORT)

add_subdirectory(Shader)
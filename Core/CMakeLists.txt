
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Core STATIC ${SOURCES})



target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Core
    PUBLIC
    Platform
    glm::glm-header-only
    Boost::uuid
    RTTR::RTTR
)
# target_compile_definitions(Core PRIVATE MENGINE_EXPORT)

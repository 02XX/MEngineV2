
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Core STATIC ${SOURCES})



target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/3rdParty/tinyexr/

${CMAKE_SOURCE_DIR}/3rdParty/tinyexr/deps/miniz/ ${CMAKE_SOURCE_DIR}/3rdParty/tinyexr/deps/nanozlib ${CMAKE_SOURCE_DIR}/3rdParty/tinyexr/deps/ZFP)
target_link_libraries(
    Core
    PUBLIC
    Platform
    glm::glm-header-only
    miniz
    # RTTR::Core
)
# target_compile_definitions(Core PRIVATE MENGINE_EXPORT)


file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Function SHARED ${SOURCES})



target_include_directories(Function PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Function
    PUBLIC
    Core
    Resource
    Platform
    EnTT::EnTT

)
target_compile_definitions(Function PRIVATE MENGINE_EXPORT)

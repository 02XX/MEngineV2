file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(MEngine ${SOURCES})
target_include_directories(MEngine PRIVATE include)



target_link_libraries(MEngine 
PUBLIC
Platform
Function
Boost.DI
)
target_compile_definitions(MEngine PRIVATE MENGINE_EXPORT)
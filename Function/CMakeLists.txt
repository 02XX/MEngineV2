
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Function STATIC ${SOURCES})
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/3rdParty/imgui")
message(STATUS "ImGui路径: ${IMGUI_DIR}") 
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS
    "${IMGUI_DIR}/*.cpp"
)

target_include_directories(Function PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include 
${CMAKE_SOURCE_DIR}/3rdParty/imgui
${CMAKE_SOURCE_DIR}/3rdParty/imgui/backends)

target_link_libraries(
    Function
    PUBLIC
    Core
    Resource
    Platform
    EnTT::EnTT
    ${IMGUI_SRC}
)
# target_compile_definitions(Function PRIVATE MENGINE_EXPORT)

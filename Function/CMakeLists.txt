
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/3rdParty/imgui")
message(STATUS "ImGui路径: ${IMGUI_DIR}") 
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
)
set(IMGUIZMO_DIR "${CMAKE_SOURCE_DIR}/3rdParty/ImGuizmo")
message(STATUS "ImGuizmo路径: ${IMGUIZMO_DIR}")
set(IMGUIZMO_SOURCES
    "${IMGUIZMO_DIR}/ImGuizmo.cpp"
    # "${IMGUIZMO_DIR}/ImSequencer.cpp"
    # "${IMGUIZMO_DIR}/ImCurveEdit.cpp"
    # "${IMGUIZMO_DIR}/ImGradient.cpp"
    # "${IMGUIZMO_DIR}/GraphEditor.cpp"
)

add_library(Function STATIC ${SOURCES} ${IMGUI_SOURCES} ${IMGUIZMO_SOURCES})



target_include_directories(Function PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include 
${CMAKE_SOURCE_DIR}/3rdParty/imgui
${CMAKE_SOURCE_DIR}/3rdParty/imgui/backends

${CMAKE_SOURCE_DIR}/3rdParty/ImGuizmo/
)

target_link_libraries(
    Function
    PUBLIC
    Core
    Resource
    Platform
    EnTT::EnTT
    SDL3::SDL3
)
# target_compile_definitions(Function PRIVATE MENGINE_EXPORT)
target_compile_definitions(Function PUBLIC IMGUI_ENABLE_DEMO_WINDOWS)
target_compile_definitions(Function PUBLIC IMGUI_ENABLE_DOCKING)

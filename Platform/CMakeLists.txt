
file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(Platform STATIC ${SOURCES})
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    message("Vulkan headers: ${Vulkan_INCLUDE_DIRS}")
    message("Vulkan libraries: ${Vulkan_LIBRARY}")
    target_link_libraries(Platform PUBLIC Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found")
endif()
target_include_directories(Platform PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include 
${CMAKE_SOURCE_DIR}/3rdParty/imgui
${CMAKE_SOURCE_DIR}/3rdParty/imgui/backends)


target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Platform
    PUBLIC
    SDL3::SDL3
    spdlog::spdlog
    GPUOpen::VulkanMemoryAllocator
    glm::glm-header-only
    magic_enum::magic_enum
    Boost.DI
    nlohmann_json::nlohmann_json
)
# target_compile_definitions(Platform PRIVATE MENGINE_EXPORT)

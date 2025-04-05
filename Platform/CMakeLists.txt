
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

target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    Platform
    PUBLIC
    SDL3::SDL3
    spdlog::spdlog_header_only
    GPUOpen::VulkanMemoryAllocator
    glm::glm-header-only
    magic_enum::magic_enum
    fmt::fmt-header-only
    Boost.DI
)
# target_compile_definitions(Platform PRIVATE MENGINE_EXPORT)

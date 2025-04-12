
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
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


# Shader
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shader/*.vert ${CMAKE_CURRENT_SOURCE_DIR}/Shader/*.frag)
find_program(GLSLC glslc REQUIRED)
foreach(shader ${SOURCES})
    get_filename_component(shader_name ${shader} NAME)
    set(output_file ${CMAKE_CURRENT_SOURCE_DIR}/Shader/${shader_name}.spv)
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${GLSLC} ${shader} -o ${output_file}
        DEPENDS ${shader}
        COMMENT "Compiling shader: ${shader_name}"
    )
    list(APPEND SHADER_SPV_FILES ${output_file})
endforeach()
add_custom_target(shader DEPENDS ${SHADER_SPV_FILES}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Resource/Shader
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Shader ${CMAKE_BINARY_DIR}/bin/Resource/Shader
    COMMENT "Copying Shader from source_folder to destination_folder..."
    VERBATIM
)

# Config
add_custom_target(
    config
    ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Config
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Config ${CMAKE_BINARY_DIR}/bin/Config
    COMMENT "Copying Config files from source_folder to destination_folder..."
    VERBATIM
)
# Material
add_custom_target(
    material
    ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Resource/Material
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Material ${CMAKE_BINARY_DIR}/bin/Resource/Material
    COMMENT "Copying Material files from source_folder to destination_folder..."
    VERBATIM
)
#UI
add_custom_target(
    ui
    ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Resource/UI
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/UI ${CMAKE_BINARY_DIR}/bin/Resource/UI
    COMMENT "Copying UI files from source_folder to destination_folder..."
    VERBATIM
)

add_dependencies(Resource shader)
add_dependencies(Resource config)
add_dependencies(Resource material)
add_dependencies(Resource ui)
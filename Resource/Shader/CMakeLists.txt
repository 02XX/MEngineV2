file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.vert ${CMAKE_CURRENT_SOURCE_DIR}/*.frag)
find_program(GLSLC glslc REQUIRED)
foreach(shader ${SOURCES})
    get_filename_component(shader_name ${shader} NAME)
    set(output_file ${CMAKE_BINARY_DIR}/bin/Shaders/${shader_name}.spv)
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND ${GLSLC} ${shader} -o ${output_file}
        DEPENDS ${shader}
        COMMENT "Compiling shader: ${shader_name}"
    )
    list(APPEND SHADER_SPV_FILES ${output_file})
endforeach()

add_custom_target(shaders DEPENDS ${SHADER_SPV_FILES})
file(GLOB_RECURSE PNG_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
find_program(GLSLC glslc REQUIRED)

add_custom_target(
    assets
    ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Assets  
    COMMAND ${CMAKE_COMMAND} -E copy ${PNG_SOURCES} ${CMAKE_BINARY_DIR}/bin/Assets/
    COMMENT "Copying only .png files from source_folder to destination_folder..."
    VERBATIM
)
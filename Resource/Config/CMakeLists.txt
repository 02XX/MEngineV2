file(GLOB_RECURSE JSON_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.json)

add_custom_target(
    config
    ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${JSON_SOURCES} ${CMAKE_BINARY_DIR}/bin/
    COMMENT "Copying *.json files from source_folder to destination_folder..."
    VERBATIM
)
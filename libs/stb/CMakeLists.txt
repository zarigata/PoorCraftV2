# libs/stb/CMakeLists.txt - stb_image library configuration

# stb_image single-file image loading library
add_library(stb_image STATIC
    stb_image.cpp
)

# Include directories
target_include_directories(stb_image
    PUBLIC
        .
)

# Export for use in main project
set_target_properties(stb_image PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

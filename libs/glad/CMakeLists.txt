# libs/glad/CMakeLists.txt - GLAD OpenGL loader configuration

# GLAD library for OpenGL function loading
add_library(glad STATIC
    src/glad.c
)

# Include directories
target_include_directories(glad
    PUBLIC
        include
)

# Link against dynamic loading library on Unix systems
if(UNIX AND NOT APPLE)
    target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
endif()

# Set C standard for GLAD
set_target_properties(glad PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Export for use in main project
set_target_properties(glad PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

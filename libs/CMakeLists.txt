# libs/CMakeLists.txt - Third-party dependencies CMake configuration

# GLFW - Window and input management
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# GLAD - OpenGL function loader
add_subdirectory(glad)

# GLM - OpenGL Mathematics (header-only)
add_subdirectory(glm)

# stb_image - Single-file image loading library
add_subdirectory(stb)

# ENet - Reliable UDP networking library
set(ENET_STATIC ON CACHE BOOL "" FORCE)
set(ENET_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(enet)

# Export library targets for use in main project
set_target_properties(glfw PROPERTIES FOLDER "Dependencies")
set_target_properties(glad PROPERTIES FOLDER "Dependencies")
set_target_properties(glm PROPERTIES FOLDER "Dependencies")
set_target_properties(stb_image PROPERTIES FOLDER "Dependencies")
set_target_properties(enet PROPERTIES FOLDER "Dependencies")

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(glfw PRIVATE /W0)
    target_compile_options(glad PRIVATE /W0)
    target_compile_options(enet PRIVATE /W0)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(glfw PRIVATE -w)
    target_compile_options(glad PRIVATE -w)
    target_compile_options(enet PRIVATE -w)
endif()

# Group dependencies in IDE
set_property(TARGET glfw PROPERTY FOLDER "Dependencies/GLFW")
set_property(TARGET glad PROPERTY FOLDER "Dependencies/GLAD")
set_property(TARGET glm PROPERTY FOLDER "Dependencies/GLM")
set_property(TARGET stb_image PROPERTY FOLDER "Dependencies/STB")
set_property(TARGET enet PROPERTY FOLDER "Dependencies/ENet")

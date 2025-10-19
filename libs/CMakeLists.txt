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

# Dear ImGui - Immediate-mode GUI library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Export library targets for use in main project
set_target_properties(glfw PROPERTIES FOLDER "Dependencies")
set_target_properties(glad PROPERTIES FOLDER "Dependencies")
set_target_properties(glm PROPERTIES FOLDER "Dependencies")
set_target_properties(stb_image PROPERTIES FOLDER "Dependencies")
set_target_properties(enet PROPERTIES FOLDER "Dependencies")
set_target_properties(imgui PROPERTIES FOLDER "Dependencies")

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(glfw PRIVATE /W0)
    target_compile_options(glad PRIVATE /W0)
    target_compile_options(enet PRIVATE /W0)
    target_compile_options(imgui PRIVATE /W0)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(glfw PRIVATE -w)
    target_compile_options(glad PRIVATE -w)
    target_compile_options(enet PRIVATE -w)
    target_compile_options(imgui PRIVATE -w)
endif()

# Group dependencies in IDE
set_property(TARGET glfw PROPERTY FOLDER "Dependencies/GLFW")
set_property(TARGET glad PROPERTY FOLDER "Dependencies/GLAD")
set_property(TARGET glm PROPERTY FOLDER "Dependencies/GLM")
set_property(TARGET stb_image PROPERTY FOLDER "Dependencies/STB")
set_property(TARGET enet PROPERTY FOLDER "Dependencies/ENet")
set_property(TARGET imgui PROPERTY FOLDER "Dependencies/ImGui")

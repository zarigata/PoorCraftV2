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
target_link_libraries(imgui PUBLIC glad)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Lua 5.4 - Scripting runtime
set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua)
file(GLOB LUA_SOURCES
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/linit.c
)

add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC ${LUA_DIR})
if(UNIX)
    target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
    target_link_libraries(lua PUBLIC m dl)
endif()

# sol2 - Header-only Lua C++ binding (no build step)
set(SOL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sol2)
target_include_directories(lua PUBLIC ${SOL2_DIR}/include)

# Export library targets for use in main project
set_target_properties(glfw PROPERTIES FOLDER "Dependencies")
set_target_properties(glad PROPERTIES FOLDER "Dependencies")
set_target_properties(glm PROPERTIES FOLDER "Dependencies")
set_target_properties(stb_image PROPERTIES FOLDER "Dependencies")
set_target_properties(enet PROPERTIES FOLDER "Dependencies")
set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
set_target_properties(lua PROPERTIES FOLDER "Dependencies")

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(glfw PRIVATE /W0)
    target_compile_options(glad PRIVATE /W0)
    target_compile_options(enet PRIVATE /W0)
    target_compile_options(imgui PRIVATE /W0)
    target_compile_options(lua PRIVATE /W0)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(glfw PRIVATE -w)
    target_compile_options(glad PRIVATE -w)
    target_compile_options(enet PRIVATE -w)
    target_compile_options(imgui PRIVATE -w)
    target_compile_options(lua PRIVATE -w)
endif()

# Group dependencies in IDE
set_property(TARGET glfw PROPERTY FOLDER "Dependencies/GLFW")
set_property(TARGET glad PROPERTY FOLDER "Dependencies/GLAD")
set_property(TARGET glm PROPERTY FOLDER "Dependencies/GLM")
set_property(TARGET stb_image PROPERTY FOLDER "Dependencies/STB")
set_property(TARGET enet PROPERTY FOLDER "Dependencies/ENet")
set_property(TARGET imgui PROPERTY FOLDER "Dependencies/ImGui")
set_property(TARGET lua PROPERTY FOLDER "Dependencies/Lua")

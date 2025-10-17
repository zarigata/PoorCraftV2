# CMakeLists.txt - Root CMake configuration for PoorCraft engine

cmake_minimum_required(VERSION 3.15)

# Project configuration
project(PoorCraft
    VERSION 0.1.0
    DESCRIPTION "Open-source Minecraft clone with multiplayer, modding, and ray tracing"
    LANGUAGES CXX
)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection
if(WIN32)
    set(PC_PLATFORM_WINDOWS ON)
elseif(UNIX AND NOT APPLE)
    set(PC_PLATFORM_LINUX ON)
elseif(APPLE)
    set(PC_PLATFORM_MACOS ON)
endif()

# Compiler flags
if(MSVC)
    # MSVC specific flags
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    # Release optimizations for MSVC
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:/O2>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    # Release optimizations for GCC/Clang
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:RelWithDebInfo>:-O2 -g>)
endif()

# Enable folders in IDEs that support it
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add subdirectories
add_subdirectory(libs)
add_subdirectory(src)

target_sources(PoorCraft PRIVATE
    ${PROJECT_SOURCE_DIR}/libs/glad/src/glad.c
)

target_include_directories(PoorCraft PRIVATE
    ${PROJECT_SOURCE_DIR}/libs/glad/include
    ${PROJECT_SOURCE_DIR}/libs/stb
)

# Assets and shaders directories (for organization)
# add_subdirectory(shaders)
# add_subdirectory(assets)

# Installation rules
install(
    DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
    DESTINATION bin
    FILES_MATCHING PATTERN "*"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
)

# Install assets and shaders directories
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
    DESTINATION share/poorcraft/assets
    FILES_MATCHING PATTERN "*"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/
    DESTINATION share/poorcraft/shaders
    FILES_MATCHING PATTERN "*"
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PoorCraftConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/PoorCraftConfigVersion.cmake"
    DESTINATION lib/cmake/PoorCraft
)

# Summary
message(STATUS "PoorCraft v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")

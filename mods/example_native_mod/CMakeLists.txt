cmake_minimum_required(VERSION 3.15)
project(example_native_mod VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create shared library
add_library(example_native_mod SHARED example_native_mod.cpp)

# Include PoorCraft headers for ModAPI
target_include_directories(example_native_mod PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Set output directory to mods folder
set_target_properties(example_native_mod PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/mods/example_native_mod
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/mods/example_native_mod
)

# Copy mod.json to output directory
add_custom_command(TARGET example_native_mod POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/mod.json
        $<TARGET_FILE_DIR:example_native_mod>/mod.json
)

# src/CMakeLists.txt - Source files CMake configuration

# Collect source files
file(GLOB_RECURSE SOURCES
    "*.cpp"
    "core/*.cpp"
    "platform/*.cpp"
    "physics/*.cpp"
    "rendering/*.cpp"
    "utils/*.cpp"
    "network/*.cpp"
    "world/*.cpp"
)

# Create executable target
add_executable(PoorCraft ${SOURCES})

add_custom_command(TARGET PoorCraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/shaders
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)

# Link against third-party libraries
target_link_libraries(PoorCraft
    PRIVATE
        glfw
        glad
        glm
        stb_image
        enet
)

# Set include directories
target_include_directories(PoorCraft
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/libs/glad/include
        ${PROJECT_SOURCE_DIR}/libs/glm
        ${PROJECT_SOURCE_DIR}/libs/stb
        ${PROJECT_SOURCE_DIR}/libs/FastNoiseLite
)

# Platform-specific linking
if(WIN32)
    # Windows specific libraries
    target_link_libraries(PoorCraft PRIVATE opengl32)
elseif(UNIX AND NOT APPLE)
    # Linux specific libraries
    target_link_libraries(PoorCraft PRIVATE
        GL
        dl
        pthread
        X11
        Xrandr
        Xinerama
        Xcursor
        Xi
    )
elseif(APPLE)
    # macOS specific libraries
    target_link_libraries(PoorCraft PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Set output directory (inherits from root CMakeLists.txt)
set_target_properties(PoorCraft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Set working directory for debugging
set_target_properties(PoorCraft PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# Compiler warnings for source files
if(MSVC)
    target_compile_options(PoorCraft PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PoorCraft PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable C++17 features
target_compile_features(PoorCraft PRIVATE cxx_std_17)

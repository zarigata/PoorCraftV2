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
    "ui/*.cpp"
    "modding/*.cpp"
    "vulkan/*.cpp"
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
        imgui
        lua
)

# Link Vulkan if available
if(Vulkan_FOUND)
    target_link_libraries(PoorCraft PRIVATE Vulkan::Vulkan)
    target_include_directories(PoorCraft PRIVATE ${Vulkan_INCLUDE_DIRS})
endif()

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
        ${CMAKE_DL_LIBS}
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

# Compile GLSL shaders to SPIR-V if glslangValidator available
if(GLSLANG_VALIDATOR)
    file(GLOB_RECURSE GLSL_SHADERS
        "${PROJECT_SOURCE_DIR}/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/shaders/*.rgen"  # Ray generation
        "${PROJECT_SOURCE_DIR}/shaders/*.rchit" # Ray closest hit
        "${PROJECT_SOURCE_DIR}/shaders/*.rmiss" # Ray miss
    )
    
    foreach(GLSL_SHADER ${GLSL_SHADERS})
        get_filename_component(SHADER_NAME ${GLSL_SHADER} NAME)
        set(SPIRV_SHADER "${PROJECT_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV_SHADER}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders"
            COMMAND ${GLSLANG_VALIDATOR} -V ${GLSL_SHADER} -o ${SPIRV_SHADER}
            DEPENDS ${GLSL_SHADER}
            COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
        )
        list(APPEND SPIRV_SHADERS ${SPIRV_SHADER})
    endforeach()
    
    add_custom_target(CompileShaders ALL DEPENDS ${SPIRV_SHADERS})
    add_dependencies(PoorCraft CompileShaders)
endif()

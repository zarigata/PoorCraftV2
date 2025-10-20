#!/bin/bash

# Shader compilation script for PoorCraft Vulkan shaders
# Compiles GLSL shaders to SPIR-V using glslc (Vulkan SDK)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SHADER_DIR="$PROJECT_DIR/shaders"

echo "=== PoorCraft Shader Compiler ==="
echo "Shader directory: $SHADER_DIR"
echo ""

# Check if glslc is available
if ! command -v glslc &> /dev/null; then
    echo "ERROR: glslc not found. Please install the Vulkan SDK."
    echo "Download from: https://vulkan.lunarg.com/"
    exit 1
fi

echo "Using glslc: $(which glslc)"
echo ""

# Function to compile a shader
compile_shader() {
    local shader_file=$1
    local output_file="${shader_file}.spv"
    
    echo "Compiling: $shader_file"
    if glslc "$shader_file" -o "$output_file"; then
        echo "  ✓ Success: $output_file"
    else
        echo "  ✗ Failed: $shader_file"
        return 1
    fi
}

# Compile basic Vulkan shaders
echo "--- Basic Shaders ---"
compile_shader "$SHADER_DIR/basic/fullscreen.vert"
compile_shader "$SHADER_DIR/basic/fullscreen.frag"
echo ""

# Compile ray tracing shaders
echo "--- Ray Tracing Shaders ---"
compile_shader "$SHADER_DIR/rt/raygen.rgen"
compile_shader "$SHADER_DIR/rt/miss.rmiss"
compile_shader "$SHADER_DIR/rt/closesthit.rchit"
echo ""

echo "=== Shader compilation complete ==="

#!/bin/bash

# setup_dependencies.sh - Unix dependency setup script for PoorCraft
# This script initializes Git submodules and downloads additional dependencies

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info "PoorCraft Dependency Setup Script"
print_info "This script will set up all required dependencies for PoorCraft"

# Check for required tools
print_info "Checking prerequisites..."

if ! command -v git &> /dev/null; then
    print_error "Git is not installed or not in PATH"
    print_error "Please install Git from https://git-scm.com/downloads"
    exit 1
fi

GIT_VERSION=$(git --version | cut -d' ' -f3)
print_success "Git version: $GIT_VERSION"

if ! command -v cmake &> /dev/null; then
    print_warning "CMake is not installed or not in PATH"
    print_warning "Some dependency checks may not work properly"
fi

# Check if we're in the correct directory (has CMakeLists.txt)
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found in current directory"
    print_error "Please run this script from the PoorCraft root directory"
    exit 1
fi

print_success "Running from correct directory"

# Initialize and update Git submodules
print_info "Setting up Git submodules..."

if [ -f ".gitmodules" ]; then
    print_info "Found .gitmodules file, initializing submodules..."

    # Check if submodules are already initialized
    if [ -d "libs/glfw/.git" ] || [ -d "libs/glm/.git" ]; then
        print_info "Submodules appear to be already initialized"
        print_info "Updating existing submodules..."
        git submodule update --init --recursive
    else
        print_info "Initializing submodules for the first time..."
        git submodule update --init --recursive
    fi

    print_success "Git submodules updated successfully"

    # Verify submodules
    print_info "Verifying submodules..."

    if [ ! -d "libs/glfw" ]; then
        print_error "GLFW submodule not found after initialization"
        print_error "Please check your internet connection and try again"
        exit 1
    fi

    if [ ! -d "libs/glm" ]; then
        print_error "GLM submodule not found after initialization"
        print_error "Please check your internet connection and try again"
        exit 1
    fi

    print_info "Initializing ENet submodule..."
    git submodule update --init --recursive libs/enet
    if [ $? -eq 0 ]; then
        print_success "ENet submodule initialized successfully"
    else
        print_error "Failed to initialize ENet submodule"
        exit 1
    fi

    print_info "Initializing ImGui submodule..."
    git submodule update --init --recursive libs/imgui
    if [ $? -eq 0 ]; then
        print_success "ImGui submodule initialized successfully"
    else
        print_error "Failed to initialize ImGui submodule"
        exit 1
    fi

    if [ ! -d "libs/imgui" ]; then
        print_error "ImGui submodule not found after initialization"
        print_error "Please check your internet connection and try again"
        exit 1
    fi

    print_info "Initializing Lua submodule..."
    git submodule update --init --recursive libs/lua
    if [ $? -eq 0 ]; then
        print_success "Lua submodule initialized successfully"
    else
        print_error "Failed to initialize Lua submodule"
        exit 1
    fi

    print_info "Initializing sol2 submodule..."
    git submodule update --init --recursive libs/sol2
    if [ $? -eq 0 ]; then
        print_success "sol2 submodule initialized successfully"
    else
        print_error "Failed to initialize sol2 submodule"
        exit 1
    fi

    print_success "All Git submodules verified"

else
    print_warning ".gitmodules file not found"
    print_warning "Manual dependency setup may be required"
fi

# Set up GLAD (OpenGL loader)
print_info "Setting up GLAD (OpenGL function loader)..."

if [ ! -d "libs/glad" ]; then
    print_info "Creating GLAD directory..."
    mkdir -p libs/glad
fi

# Check if GLAD files already exist
if [ -f "libs/glad/include/glad/glad.h" ] && [ -f "libs/glad/src/glad.c" ]; then
    print_info "GLAD files already exist, skipping download"
else
    print_info "GLAD files not found, please generate them manually:"
    print_info ""
    print_info "1. Go to https://glad.dav1d.de/"
    print_info "2. Set 'API' to 'gl' (version 4.6)"
    print_info "3. Set 'Profile' to 'Core'"
    print_info "4. Set 'Options' to 'Generate a loader'"
    print_info "5. Click 'Generate'"
    print_info "6. Download the ZIP file"
    print_info "7. Extract 'include/glad/glad.h' to libs/glad/include/glad/"
    print_info "8. Extract 'src/glad.c' to libs/glad/src/"
    print_info ""
    print_warning "GLAD setup requires manual intervention"
    print_warning "Please complete the steps above and run this script again"
    exit 1
fi

print_success "GLAD setup completed"

# Set up stb_image (single-file image library)
print_info "Setting up stb_image..."

if [ ! -d "libs/stb" ]; then
    print_info "Creating stb directory..."
    mkdir -p libs/stb
fi

# Check if stb_image.h already exists
if [ -f "libs/stb/stb_image.h" ]; then
    print_info "stb_image.h already exists, skipping download"
else
    print_info "Downloading stb_image.h..."

    # Download stb_image.h from GitHub
    if command -v curl &> /dev/null; then
        curl -L -o libs/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    elif command -v wget &> /dev/null; then
        wget -O libs/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    else
        print_error "Neither curl nor wget is available for downloading"
        print_error "Please manually download stb_image.h from:"
        print_error "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        print_error "And place it in libs/stb/stb_image.h"
        exit 1
    fi

    # Create wrapper .cpp file for CMake
    cat > libs/stb/stb_image.cpp << 'EOF'
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
EOF

    print_success "stb_image.h downloaded and wrapper created"
fi

# Verify stb_image setup
if [ -f "libs/stb/stb_image.h" ] && [ -f "libs/stb/stb_image.cpp" ]; then
    print_success "stb_image setup verified"
else
    print_error "stb_image setup failed"
    exit 1
fi

# Verify CMakeLists.txt files exist
print_info "Verifying CMake configuration files..."

REQUIRED_CMAKE_FILES=(
    "libs/CMakeLists.txt"
    "libs/glad/CMakeLists.txt"
    "libs/stb/CMakeLists.txt"
    "src/CMakeLists.txt"
)

for cmake_file in "${REQUIRED_CMAKE_FILES[@]}"; do
    if [ -f "$cmake_file" ]; then
        print_success "Found $cmake_file"
    else
        print_error "Missing $cmake_file"
        print_error "Please ensure all required files are present"
        exit 1
    fi
done

# Check for optional tools
print_info "Checking for optional development tools..."

if command -v clang-format &> /dev/null; then
    CLANG_FORMAT_VERSION=$(clang-format --version | cut -d' ' -f3)
    print_success "clang-format version: $CLANG_FORMAT_VERSION"
else
    print_warning "clang-format not found - code formatting will not be available"
    print_warning "Install clang-format for automatic code formatting"
fi

if command -v ccache &> /dev/null; then
    CCACHE_VERSION=$(ccache --version | head -n1)
    print_success "ccache found: $CCACHE_VERSION"
    print_info "Consider using ccache for faster recompilation"
else
    print_info "ccache not found - builds may be slower"
    print_info "Install ccache for faster recompilation (optional)"
fi

# Show summary
print_info ""
print_info "=== Dependency Setup Summary ==="
print_success "Git submodules: Initialized"
print_success "GLFW: Ready"
print_success "GLM: Ready"
print_success "GLAD: Ready"
print_success "stb_image: Ready"
print_success "CMake configuration: Verified"
print_info ""

# Show next steps
print_info "Next steps:"
print_info "1. Run the build script: ./scripts/build.sh"
print_info "2. Or build manually: cmake -B build && cmake --build build"
print_info "3. See docs/BUILD.md for detailed build instructions"
print_info "4. Check docs/CONTRIBUTING.md for development guidelines"

print_success "Dependency setup completed successfully!"
print_info "You can now build the PoorCraft engine"

# Make the script executable
chmod +x "$0"

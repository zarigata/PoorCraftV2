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

# Options
FORCE_FLAG=false
DOWNLOAD_GLAD=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --force)
            FORCE_FLAG=true
            shift
            ;;
        --download-glad)
            DOWNLOAD_GLAD=true
            shift
            ;;
        --help|-h)
            cat <<'USAGE'
Usage: ./scripts/setup_dependencies.sh [--force] [--download-glad]

  --force           Retry submodule fetch with git's --force flag
  --download-glad   Attempt to download pre-generated GLAD files from mirror
  --help            Show this help message
USAGE
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

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

check_network() {
    print_info "Checking network connectivity..."
    if command -v ping >/dev/null 2>&1; then
        if ! ping -c 1 github.com >/dev/null 2>&1; then
            print_error "Unable to reach github.com via ping"
            print_error "Please verify your internet connection or firewall settings"
            exit 1
        fi
    elif command -v getent >/dev/null 2>&1; then
        if ! getent hosts github.com >/dev/null 2>&1; then
            print_error "DNS lookup for github.com failed"
            exit 1
        fi
    else
        print_warning "Neither ping nor getent available for connectivity check; proceeding without validation"
    fi
    print_success "Network connectivity verified"
}

directory_has_content() {
    local dir="$1"
    if [ ! -d "$dir" ]; then
        return 1
    fi
    if [ -z "$(find "$dir" -mindepth 1 -maxdepth 1 ! -name '.git' ! -name '.gitignore' ! -name '.gitkeep' 2>/dev/null)" ]; then
        return 1
    fi
    return 0
}

verify_file() {
    local file_path="$1"
    if [ ! -f "$file_path" ]; then
        print_error "Expected file missing: $file_path"
        exit 1
    fi
}

print_submodule_progress() {
    local index="$1"
    local total="$2"
    local name="$3"
    print_info "Fetching ${name}... ${index}/${total}"
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

check_network

SUBMODULE_ARGS=(--init --recursive)
if [ "$FORCE_FLAG" = true ]; then
    SUBMODULE_ARGS+=(--force)
fi

# Initialize and update Git submodules
print_info "Setting up Git submodules..."

if [ -f ".gitmodules" ]; then
    print_info "Found .gitmodules file, initializing submodules..."

    local_progress=0
    total_submodules=6

    for submodule in libs/glfw libs/glm libs/enet libs/imgui libs/lua libs/sol2; do
        local_progress=$((local_progress + 1))
        print_submodule_progress "$local_progress" "$total_submodules" "$submodule"
        if ! git submodule update "${SUBMODULE_ARGS[@]}" "$submodule"; then
            print_error "Failed to initialize submodule: $submodule"
            print_error "Try rerunning with --force or inspect .gitmodules configuration"
            exit 1
        fi
    done

    print_success "Git submodules updated successfully"

    # Verify submodules
    print_info "Verifying submodules..."

    verify_file "libs/glfw/CMakeLists.txt"
    directory_has_content "libs/glfw" || { print_error "GLFW directory is empty after initialization"; exit 1; }

    verify_file "libs/glm/CMakeLists.txt"
    directory_has_content "libs/glm" || { print_error "GLM directory is empty after initialization"; exit 1; }

    verify_file "libs/enet/CMakeLists.txt"
    directory_has_content "libs/enet" || { print_error "ENet directory is empty after initialization"; exit 1; }

    verify_file "libs/imgui/imgui.cpp"
    directory_has_content "libs/imgui" || { print_error "ImGui directory is empty after initialization"; exit 1; }

    verify_file "libs/lua/lua.h"
    directory_has_content "libs/lua" || { print_error "Lua directory is empty after initialization"; exit 1; }

    verify_file "libs/sol2/include/sol/sol.hpp"
    directory_has_content "libs/sol2" || { print_error "sol2 directory is empty after initialization"; exit 1; }

    print_success "All Git submodules verified"

else
    print_warning ".gitmodules file not found"
    print_warning "Manual dependency setup may be required"
fi

# Set up GLAD (OpenGL loader)
print_info "Setting up GLAD (OpenGL function loader)..."

if [ ! -d "libs/glad" ]; then
    print_info "Creating GLAD directory..."
    mkdir -p libs/glad/include/glad
    mkdir -p libs/glad/include/KHR
    mkdir -p libs/glad/src
fi

# Check if GLAD files already exist
if [ -f "libs/glad/include/glad/glad.h" ] && [ -f "libs/glad/src/glad.c" ]; then
    print_info "GLAD files already exist, skipping generation"
else
    if [ "$DOWNLOAD_GLAD" = true ]; then
        print_info "Attempting to download GLAD loader from mirror..."
        GLAD_BASE_URL="https://raw.githubusercontent.com/Dav1dde/glad/master"
        if command -v curl &> /dev/null; then
            curl -fL -o libs/glad/include/glad/glad.h "$GLAD_BASE_URL/include/glad/glad.h"
            curl -fL -o libs/glad/include/KHR/khrplatform.h "$GLAD_BASE_URL/include/KHR/khrplatform.h"
            curl -fL -o libs/glad/src/glad.c "$GLAD_BASE_URL/src/glad.c"
        elif command -v wget &> /dev/null; then
            wget -O libs/glad/include/glad/glad.h "$GLAD_BASE_URL/include/glad/glad.h"
            wget -O libs/glad/include/KHR/khrplatform.h "$GLAD_BASE_URL/include/KHR/khrplatform.h"
            wget -O libs/glad/src/glad.c "$GLAD_BASE_URL/src/glad.c"
        else
            print_warning "Neither curl nor wget available to download GLAD"
        fi
    fi

    if [ -f "libs/glad/include/glad/glad.h" ] && [ -f "libs/glad/src/glad.c" ]; then
        print_success "GLAD files downloaded successfully"
    else
        print_warning "GLAD files not found"
        print_info "1. Go to https://glad.dav1d.de/"
        print_info "2. Set 'API' to 'gl' (version 4.6)"
        print_info "3. Set 'Profile' to 'Core'"
        print_info "4. Set 'Options' to 'Generate a loader'"
        print_info "5. Click 'Generate' and download the ZIP file"
        print_info "6. Extract 'include/glad/glad.h' to libs/glad/include/glad/"
        print_info "7. Extract 'include/KHR/khrplatform.h' to libs/glad/include/KHR/"
        print_info "8. Extract 'src/glad.c' to libs/glad/src/"
        print_error "GLAD setup requires manual intervention or rerun with --download-glad"
        exit 1
    fi
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
if git submodule status --recursive >/dev/null 2>&1; then
    git submodule status --recursive | while read -r line; do
        print_success "Submodule: $line"
    done
else
    print_warning "Unable to list submodule status"
fi
print_success "GLFW: Verified"
print_success "GLM: Verified"
print_success "GLAD: Ready"
print_success "stb_image: Ready"
print_success "CMake configuration: Verified"
print_info ""

# Show next steps
print_info "Next steps:"
print_info "1. Run the build script: ./scripts/build.sh"
print_info "2. Or build manually: cmake -B build && cmake --build build"
print_info "3. Run ./scripts/verify_build.sh after building to validate artifacts"
print_info "4. See docs/BUILD.md for detailed build instructions"
print_info "5. Check docs/CONTRIBUTING.md for development guidelines"

print_success "Dependency setup completed successfully!"
print_info "You can now build the PoorCraft engine"

# Make the script executable
chmod +x "$0"

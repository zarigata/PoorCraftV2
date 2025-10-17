#!/bin/bash

# build.sh - Unix build script for PoorCraft engine
# This script automates the build process for Linux and macOS

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

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
GENERATOR=""
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CLEAN_BUILD=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug           Build in Debug mode (default: Release)"
            echo "  --release         Build in Release mode"
            echo "  --clean           Clean build directory before building"
            echo "  --verbose         Enable verbose output"
            echo "  --generator GEN   Specify CMake generator (default: auto)"
            echo "  --jobs NUM        Number of parallel jobs (default: auto)"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

print_info "PoorCraft Build Script"
print_info "Build Type: $BUILD_TYPE"
print_info "Build Directory: $BUILD_DIR"
print_info "Parallel Jobs: $JOBS"

# Check for required tools
print_info "Checking prerequisites..."

if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed or not in PATH"
    print_error "Please install CMake 3.15 or higher"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
print_success "CMake version: $CMAKE_VERSION"

if ! command -v git &> /dev/null; then
    print_warning "Git is not installed or not in PATH"
    print_warning "Some features may not work properly"
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    print_info "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

# Change to build directory
cd "$BUILD_DIR"

# Configure with CMake
print_info "Configuring with CMake..."

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
)

if [ -n "$GENERATOR" ]; then
    CMAKE_ARGS+=(-G "$GENERATOR")
fi

if [ "$VERBOSE" = true ]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

print_info "CMake arguments: ${CMAKE_ARGS[*]}"

if ! cmake .. "${CMAKE_ARGS[@]}"; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "CMake configuration completed"

# Build the project
print_info "Building project..."

BUILD_ARGS=(
    --build .
    --config "$BUILD_TYPE"
    --parallel "$JOBS"
)

if [ "$VERBOSE" = true ]; then
    BUILD_ARGS+=(--verbose)
fi

if ! cmake "${BUILD_ARGS[@]}"; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Show build results
print_info "Build summary:"
echo "  Build Type: $BUILD_TYPE"
echo "  Output Directory: $(pwd)/bin"
echo "  Executable: $(pwd)/bin/PoorCraft"

# Check if executable was created
if [ -f "bin/PoorCraft" ]; then
    EXECUTABLE_SIZE=$(du -h "bin/PoorCraft" | cut -f1)
    print_success "Executable created successfully ($EXECUTABLE_SIZE)"

    # Make executable if on Linux
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        chmod +x "bin/PoorCraft"
    fi
else
    print_warning "Executable not found in expected location"
fi

# Show next steps
echo ""
print_info "Next steps:"
echo "  1. Run the engine: ./bin/PoorCraft"
echo "  2. See docs/BUILD.md for detailed build documentation"
echo "  3. Check docs/CONTRIBUTING.md for development guidelines"

print_success "Build script completed successfully!"

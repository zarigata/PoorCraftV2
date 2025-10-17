# Build Documentation

This document provides comprehensive build instructions for the PoorCraft game engine across all supported platforms.

## Prerequisites

### Required Software

- **CMake 3.15** or higher ([Download](https://cmake.org/download/))
- **C++17 compatible compiler**:
  - Windows: Visual Studio 2019+ or MinGW-w64
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 11+ or Clang 10+
- **Git** for dependency management ([Download](https://git-scm.com/downloads))

### Optional Software

- **Ninja** build system for faster builds ([Download](https://ninja-build.org/))
- **ccache** for faster recompilation ([Install](https://ccache.dev/))
- **clang-format** for code formatting ([Install](https://clang.llvm.org/docs/ClangFormat.html))

## Dependency Setup

### Using Git Submodules (Recommended)

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/yourusername/PoorCraft.git
cd PoorCraft

# If you forgot --recursive, initialize submodules manually
git submodule update --init --recursive
```

### Manual Dependency Setup (Alternative)

If you prefer not to use submodules:

1. **GLFW**: Download from https://github.com/glfw/glfw/releases
2. **GLM**: Download from https://github.com/g-truc/glm/releases
3. **GLAD**: Generate from https://glad.dav1d.de/ with OpenGL 4.6 Core
4. **stb_image**: Download `stb_image.h` from https://github.com/nothings/stb

## Platform-Specific Setup

### Windows

#### Visual Studio 2019/2022

1. **Install Visual Studio** with C++ development tools
2. **Install CMake** and ensure it's in your PATH
3. **Open Command Prompt** or PowerShell

```cmd
# Navigate to project directory
cd PoorCraft

# Configure with Visual Studio generator
cmake -B build -G "Visual Studio 16 2019" -A x64

# Build the project
cmake --build build --config Release

# Run the engine
build\bin\Release\PoorCraft.exe
```

#### MinGW-w64 (GCC)

1. **Install MinGW-w64** from https://www.mingw-w64.org/
2. **Ensure GCC is in your PATH**

```cmd
# Configure with MinGW Makefiles
cmake -B build -G "MinGW Makefiles"

# Build the project
cmake --build build --config Release

# Run the engine
build\bin\PoorCraft.exe
```

### Linux

#### Ubuntu/Debian

```bash
# Install build dependencies
sudo apt update
sudo apt install build-essential cmake git

# Optional: Install Ninja for faster builds
sudo apt install ninja-build

# Navigate to project directory
cd PoorCraft

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run the engine
build/bin/PoorCraft
```

#### Fedora/CentOS/RHEL

```bash
# Install build dependencies
sudo dnf install gcc-c++ cmake git make

# Optional: Install Ninja for faster builds
sudo dnf install ninja-build

# Configure and build (same as Ubuntu)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### Arch Linux

```bash
# Install build dependencies
sudo pacman -S base-devel cmake git

# Optional: Install Ninja for faster builds
sudo pacman -S ninja

# Configure and build (same as Ubuntu)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### Advanced Linux Options

```bash
# Use Ninja build system for faster builds
cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Use ccache for faster recompilation
cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Release

# Debug build with symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build with optimizations
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### macOS

#### Xcode

1. **Install Xcode** from the App Store
2. **Install CMake** via Homebrew or download

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake

# Navigate to project directory
cd PoorCraft

# Configure for Xcode
cmake -B build -G Xcode

# Build the project (opens Xcode)
cmake --build build --config Release

# Or build from command line
xcodebuild -project build/PoorCraft.xcodeproj -configuration Release

# Run the engine
build/Release/PoorCraft
```

#### Command Line (Makefile)

```bash
# Configure with Unix Makefiles
cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build -j$(sysctl -n hw.ncpu)

# Run the engine
build/bin/PoorCraft
```

## CMake Configuration Options

### Build Type

```bash
# Debug build (default)
cmake -B build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# RelWithDebInfo (release with debug info)
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

# MinSizeRel (minimum size release)
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```

### Compiler Selection

```bash
# Use Clang on Linux/macOS
cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Use GCC on Linux
cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc

# Use a specific compiler version
cmake -B build -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_C_COMPILER=clang-12
```

### Feature Flags

```bash
# Enable/disable specific features
cmake -B build -DENABLE_TESTS=ON -DENABLE_PROFILING=ON

# Set installation prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local

# Use static linking
cmake -B build -DBUILD_SHARED_LIBS=OFF
```

### Dependency Options

```bash
# Specify custom dependency paths
cmake -B build -DGLFW_ROOT=/path/to/glfw -DGLM_ROOT=/path/to/glm

# Use system packages instead of vendored libraries
cmake -B build -DUSE_SYSTEM_GLFW=ON -DUSE_SYSTEM_GLM=ON
```

## Building from IDE

### Visual Studio Code

1. Install the **CMake Tools** extension
2. Open the project folder in VS Code
3. Use **Ctrl+Shift+P** > "CMake: Configure"
4. Select your preferred build configuration
5. Use **Ctrl+Shift+P** > "CMake: Build"
6. Use **Ctrl+Shift+P** > "CMake: Run Without Debugging" to run

### CLion (JetBrains)

1. Open the project folder in CLion
2. CLion will automatically detect CMake configuration
3. Use **Build** > **Build Project** to build
4. Use **Run** > **Run** to execute

### Visual Studio

1. Open the generated `.sln` file in the build directory
2. Select your preferred configuration (Debug/Release)
3. Use **Build** > **Build Solution**
4. Use **Debug** > **Start Debugging** to run

## Troubleshooting

### Common Build Errors

#### "CMake 3.15 or higher required"
**Solution**: Update CMake to version 3.15 or higher.

#### "C++ compiler not found"
**Solution**: Ensure a C++17 compatible compiler is installed and in your PATH.

#### "OpenGL headers not found"
**Solution**: Install OpenGL development packages:
```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev

# Fedora
sudo dnf install mesa-libGL-devel

# Arch Linux
sudo pacman -S mesa
```

#### "GLFW not found"
**Solution**: Ensure GLFW is properly initialized:
```bash
git submodule update --init --recursive
```

#### "Permission denied" (macOS)
**Solution**: Allow apps from unidentified developers or build from command line.

### Performance Issues

#### Slow compilation
**Solution**:
```bash
# Use Ninja build system
cmake -B build -GNinja

# Use ccache for faster recompilation
cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

# Use parallel jobs
cmake --build build -j$(nproc)
```

#### Large binary size
**Solution**:
```bash
# Use MinSizeRel build type
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel

# Enable link-time optimization
cmake -B build -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Runtime Issues

#### "Failed to create OpenGL context"
**Solution**:
- Update graphics drivers
- Ensure OpenGL 4.6 compatibility
- Check GPU requirements in README

#### "Configuration file not found"
**Solution**: The engine creates a default `config.ini` file on first run.

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]

    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive

    - name: Configure
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release

    - name: Build
      run: cmake --build build --config Release
```

## Distribution

### Creating Release Binaries

```bash
# Create release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Install to system (optional)
sudo cmake --install build

# Create archive for distribution
cd build
tar -czf PoorCraft-linux-x64.tar.gz bin/
```

## Getting Help

If you encounter issues not covered in this documentation:

1. Check the [GitHub Issues](https://github.com/yourusername/PoorCraft/issues) page
2. Search existing discussions and documentation
3. Create a new issue with detailed information about your setup and error messages

## Advanced Configuration

For advanced users, see the [CMake documentation](https://cmake.org/documentation/) for additional configuration options and toolchain files.

# PoorCraft

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-green.svg)](https://isocpp.org/)

PoorCraft is an open-source Minecraft clone built from the ground up with modern C++ and OpenGL. The project aims to create a feature-rich voxel-based game engine with multiplayer support, modding capabilities, and advanced rendering features like ray tracing.

## 🌟 Features

- **Cross-platform compatibility** (Windows, Linux, macOS)
- **Modern OpenGL rendering pipeline** with GLSL shaders
- **Voxel-based world generation** with multiple biomes
- **32x32 pixel art skin system** for character customization
- **Multiplayer networking** infrastructure
- **Modding API** for community content
- **Advanced graphics features** (ray tracing, shadows, lighting)

## 📋 System Requirements

### Build Requirements
- **CMake 3.15** or higher
- **C++17 compatible compiler**:
  - Windows: Visual Studio 2019+ or MinGW-w64
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 11+ or Clang 10+
- **Git** for dependency management

### Runtime Requirements
- OpenGL 4.6 compatible graphics card
- 8GB RAM minimum, 16GB recommended
- Modern CPU with SSE4.1 support

## 🚀 Quick Start

### Windows (Visual Studio)

```bash
# Clone the repository
git clone https://github.com/yourusername/PoorCraft.git
cd PoorCraft

# Initialize submodules
git submodule update --init --recursive

# Configure with CMake
cmake -B build -G "Visual Studio 16 2019"

# Build the project
cmake --build build --config Release

# Run the engine
build\bin\Release\PoorCraft.exe
```

### Windows (MinGW)

```bash
# Configure with MinGW Makefiles
cmake -B build -G "MinGW Makefiles"

# Build and run
cmake --build build --config Release
build\bin\PoorCraft.exe
```

### Linux (Ubuntu/Debian)

```bash
# Install build dependencies
sudo apt update
sudo apt install build-essential cmake git

# Clone and build
git clone https://github.com/yourusername/PoorCraft.git
cd PoorCraft
git submodule update --init --recursive

# Configure and build
cmake -B build
cmake --build build -j$(nproc)

# Run the engine
build/bin/PoorCraft
```

### Linux (Fedora/CentOS)

```bash
# Install build dependencies
sudo dnf install gcc-c++ cmake git make

# Build process (same as Ubuntu)
cmake -B build
cmake --build build -j$(nproc)
```

### Linux (Arch Linux)

```bash
# Install build dependencies
sudo pacman -S base-devel cmake git

# Build process (same as Ubuntu)
cmake -B build
cmake --build build -j$(nproc)
```

### macOS

```bash
# Install CMake (if not already installed)
brew install cmake

# Clone and build
git clone https://github.com/yourusername/PoorCraft.git
cd PoorCraft
git submodule update --init --recursive

# Configure and build
cmake -B build -G "Unix Makefiles"
cmake --build build -j$(sysctl -n hw.ncpu)

# Run the engine
build/bin/PoorCraft
```

## 📁 Project Structure

```
PoorCraft/
├── CMakeLists.txt          # Root CMake configuration
├── README.md              # This file
├── LICENSE                # MIT License
├── .gitignore             # Git ignore patterns
├── include/               # Public header files
│   └── poorcraft/         # Engine namespace
│       ├── core/          # Core systems (Logger, Config)
│       └── platform/      # Platform abstraction
├── src/                   # Implementation files
│   ├── core/              # Core system implementations
│   ├── platform/          # Platform-specific code
│   └── CMakeLists.txt     # Source CMake configuration
├── libs/                  # Third-party dependencies
│   ├── glfw/              # Window and input management
│   ├── glad/              # OpenGL function loader
│   ├── glm/               # OpenGL Mathematics
│   ├── stb/               # Single-file libraries
│   └── CMakeLists.txt     # Dependencies CMake config
├── assets/                # Game resources
│   ├── textures/          # Block and entity textures
│   ├── models/            # 3D models
│   ├── sounds/            # Audio files
│   ├── fonts/             # UI fonts
│   └── README.md          # Asset organization guide
├── shaders/               # GLSL shader files
│   ├── basic/             # Simple shaders
│   ├── terrain/           # World rendering shaders
│   ├── entity/            # Entity rendering shaders
│   ├── ui/                # UI rendering shaders
│   └── README.md          # Shader organization guide
├── docs/                  # Documentation
│   ├── BUILD.md           # Detailed build instructions
│   ├── ARCHITECTURE.md    # Project architecture
│   ├── API.md             # API reference
│   └── CONTRIBUTING.md    # Contribution guidelines
└── scripts/               # Build and utility scripts
    ├── build.sh           # Unix build script
    ├── build.bat          # Windows build script
    └── setup_dependencies.sh # Dependency setup
```

## 🔧 Dependencies

PoorCraft uses the following third-party libraries:

- **[GLFW](https://github.com/glfw/glfw)** - Window creation and input handling
- **[GLAD](https://glad.dav1d.de/)** - OpenGL function loading
- **[GLM](https://github.com/g-truc/glm)** - OpenGL Mathematics library
- **[stb_image](https://github.com/nothings/stb)** - Image loading library

All dependencies are included as Git submodules for easy setup.

## ⚙️ Configuration

The engine uses a configuration file (`config.ini`) for runtime settings:

```ini
[Graphics]
width=1280
height=720
fullscreen=false
vsync=true
fov=90

[Audio]
master_volume=1.0
music_volume=0.7
sound_volume=0.8

[Controls]
mouse_sensitivity=1.0
invert_y=false

[Gameplay]
render_distance=8
difficulty=normal

[Network]
default_port=25565
timeout=5000

[Engine]
log_level=info
max_fps=144
```

## 🛠️ Development

### Building from Source

See the [Build Documentation](docs/BUILD.md) for detailed build instructions and troubleshooting.

### Code Style

The project uses `clang-format` for consistent code formatting. A `.clang-format` configuration file is included in the repository.

### Contributing

Please read the [Contributing Guidelines](docs/CONTRIBUTING.md) before making contributions.

## 📚 Documentation

- **[Build Guide](docs/BUILD.md)** - Detailed build instructions
- **[Architecture](docs/ARCHITECTURE.md)** - Project structure and design
- **[API Reference](docs/API.md)** - Core systems API documentation
- **[Contributing](docs/CONTRIBUTING.md)** - How to contribute to the project

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by the original Minecraft by Mojang Studios
- Built with modern C++ and OpenGL best practices
- Thanks to the open-source community for excellent libraries

## 🔗 Links

- **Repository**: https://github.com/yourusername/PoorCraft
- **Issues**: https://github.com/yourusername/PoorCraft/issues
- **Discussions**: https://github.com/yourusername/PoorCraft/discussions

---

**Happy crafting!** 🧱⛏️

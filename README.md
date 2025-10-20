# PoorCraft

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-green.svg)](https://isocpp.org/)

PoorCraft is an open-source Minecraft clone built from the ground up with modern C++ and OpenGL. The project aims to create a feature-rich voxel-based game engine with multiplayer support, modding capabilities, and advanced rendering features like ray tracing.

## 🌟 Features

### Currently Implemented
- **Cross-platform compatibility** (Windows, Linux, macOS)
- **Window management** with multi-monitor support via GLFW
- **Keyboard, mouse, and gamepad input** handling
- **Fixed timestep game loop** with FPS limiting
- **Event system** for engine-wide communication
- **Resource management** with caching and async loading
- **Memory management** utilities (tracking and pooling)
- **Comprehensive logging** system with file output
- **Configuration management** with INI file support
- **Platform abstraction** layer for OS-specific operations
- **Noise-driven world generation** featuring five biomes, biome blending, caves, ores, trees, cactus, tall grass, and flowers
- **Modding system** with dual native/Lua plugin support, comprehensive mod API, hot-reload, event hooks, and example mods
- **Advanced rendering system** with dynamic lighting (sky light, block light, ambient occlusion), transparent water with animation, particle effects, procedural sky with day-night cycle, and texture optimizations

### Planned Features
- **32x32 pixel art skin system** for character customization
- **Ray tracing support** (Phase 10) with Vulkan integration

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

# Initialize submodules and setup dependencies
git submodule update --init --recursive
# Verify submodules fetched correctly
ls libs/glfw/CMakeLists.txt
ls libs/imgui/imgui.cpp
scripts\setup_dependencies.bat

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
git submodule update --init --recursive
# Verify submodules fetched correctly
ls libs/glfw/CMakeLists.txt
ls libs/imgui/imgui.cpp
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
# Verify submodules fetched correctly
ls libs/glfw/CMakeLists.txt
ls libs/imgui/imgui.cpp
./scripts/setup_dependencies.sh

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
# Verify submodules fetched correctly
ls libs/glfw/CMakeLists.txt
ls libs/imgui/imgui.cpp
./scripts/setup_dependencies.sh

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
│   ├── stb/               # Image loading utilities
│   ├── FastNoiseLite/     # Noise generation library
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

**⚠️ Build issues?**
If `cmake --build build` reports success but `build/bin/PoorCraft` is missing:
1. Verify submodules: `ls libs/glfw/CMakeLists.txt`
2. Reinitialize: `git submodule update --init --recursive --force`
3. Rerun setup: `./scripts/setup_dependencies.sh`
4. Clean and rebuild: `rm -rf build && cmake -B build && cmake --build build`
See `docs/BUILD.md` and `docs/TROUBLESHOOTING.md` for full guidance.

## 🔧 Dependencies

PoorCraft uses the following third-party libraries:

- **[GLFW](https://github.com/glfw/glfw)** - Window creation and input handling (Git submodule)
- **[GLAD](https://glad.dav1d.de/)** - OpenGL 4.6 Core function loading (generated)
- **[GLM](https://github.com/g-truc/glm)** - OpenGL Mathematics library (Git submodule)
- **[stb_image](https://github.com/nothings/stb)** - Image loading library (downloaded)
- **[FastNoiseLite](https://github.com/Auburn/FastNoiseLite)** - Deterministic noise generation (vendored)
- **[Lua 5.4](https://github.com/lua/lua)** - Scripting runtime for mods (Git submodule)
- **[sol2](https://github.com/ThePhD/sol2)** - Modern C++17 Lua binding library (Git submodule)

GLFW and GLM are included as Git submodules. GLAD files need to be generated from https://glad.dav1d.de/ with OpenGL 4.6 Core profile, and stb_image.h needs to be downloaded. Use the provided `setup_dependencies` scripts to automate this process.

**Dependency checklist**
- ✅ Git submodules initialized (`ls libs/glfw/CMakeLists.txt`)
- ✅ GLAD loader present (`libs/glad/include/glad/glad.h`)
- ✅ stb headers present (`libs/stb/stb_image.h`)
- ✅ Build directory configured (`cmake -B build`)
- ✅ Executable produced (`build/bin/PoorCraft` or `build/bin/Release/PoorCraft.exe`)

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

## 🌍 World Generation

PoorCraft ships with a fully procedural overworld powered by FastNoiseLite:

- **Biomes**: Plains, Mountains, Snow, Desert, and Jungle each provide distinct surface blocks, vegetation, and features.
- **Blending**: Neighboring biome weights are dithered for mottled transitions at borders.
- **Terrain**: Layered heightmaps create rolling hills and cliffs, undercut by noise-driven caves.
- **Decorations**: Trees, cactus, tall grass, and flowers spawn deterministically based on biome definitions.
- **Ores**: Coal, iron, gold, and diamond veins generate via configurable clustered attempts.

Tune generation with the `[World]` section of `config.ini`:

- `world_seed`
- `biome_scale`
- `cave_density`
- `ore_frequency`
- `tree_density`

## 🔌 Modding

PoorCraft features a comprehensive server-side modding system supporting both native C++ plugins and Lua 5.4 scripts.

### Features
- **Dual Plugin System**: Native C++ plugins for performance, Lua scripts for flexibility
- **Server-Side Only**: Mods run on authoritative server, changes synced to clients automatically
- **Mod API**: Block registration, entity spawning, event subscription, world modification, config access, logging
- **Event Hooks**: Block placed/broken, entity spawned/destroyed, player interact, chunk generated, player join/leave
- **Hot-Reload**: Automatic reload on file changes (debug builds), rapid development workflow
- **Mod Discovery**: Scan mods/ directory for mod.json manifests, validate API version and dependencies
- **Load Ordering**: Dependency resolution with topological sort, load priority for fine control
- **Example Mods**: example_native_mod (block registration), example_lua_mod (entity spawning), event_logger_mod (event monitoring)

### Quick Start

**Create a Lua Mod:**
```lua
-- mods/my_mod/init.lua
print("My mod loading...")

-- Register custom block
local blockId = Block.registerBlock({
    name = "my_block",
    isSolid = true,
    textureName = "stone",
    hardness = 2.0
})

-- Subscribe to events
EventBus.subscribe("PlayerJoined", function(event)
    print("Welcome " .. event.playerName .. "!")
end)
```

**Create mod.json:**
```json
{
  "id": "my_mod",
  "name": "My Mod",
  "version": "1.0.0",
  "author": "YourName",
  "description": "My awesome mod",
  "apiVersion": 1,
  "dependencies": [],
  "loadPriority": 100,
  "type": "lua",
  "entry": "init.lua"
}
```

See **[Modding Guide](docs/MODDING.md)** for complete documentation.

## 🛠️ Development

### Current Status

**Completed Systems:**
- ✅ Core (Logger, Config, Platform)
- ✅ Window (GLFW integration, multi-monitor support)
- ✅ Input (Keyboard, mouse, gamepad)
- ✅ Game Loop (Fixed timestep, FPS limiting)
- ✅ Events (Pub/sub system, event types)
- ✅ Resources (Loading, caching, async support)
- ✅ Memory (Tracking, pooling)
- ✅ Modding System (Native plugins, Lua scripts, ModAPI, hot-reload)

**Next Steps:**
- 🔲 Advanced rendering (lighting, water, particles, sky)
- 🔲 Audio system
- 🔲 World persistence (save/load)

### Building from Source

See the [Build Documentation](docs/BUILD.md) for detailed build instructions and troubleshooting.

### Code Style

The project uses `clang-format` for consistent code formatting. A `.clang-format` configuration file is included in the repository.

### Contributing

Please read the [Contributing Guidelines](docs/CONTRIBUTING.md) before making contributions.

## 🎨 Rendering Features

### Lighting System
- **Sky Light**: Sunlight from above (0-15 levels), flood-fill propagation from top
- **Block Light**: Emissive blocks like torches/lava (0-15 levels), flood-fill from light sources
- **Ambient Occlusion**: Per-vertex corner darkening (0-3 occluded), enhances depth perception
- **Smooth Lighting**: Per-vertex light interpolation for smooth gradients
- **Directional Sun Light**: Calculated from time of day, affects diffuse lighting

### Water Rendering
- **Transparent Rendering**: Alpha blending with proper depth sorting
- **Wave Animation**: Sin/cos vertex displacement for subtle wave motion
- **Flow Animation**: Time-based UV offset for flowing water appearance
- **Water Tint**: Light blue color with configurable transparency
- **Proper Depth Handling**: Depth test enabled, depth write disabled for correct transparency

### Particle System
- **Block Breaking Particles**: 8-16 fragments with block texture, random velocities
- **Explosion Effects**: Radial fire/smoke particles with configurable radius
- **Particle Emitters**: Continuous effects (fire, smoke, water splash)
- **Instanced Rendering**: One draw call for all particles (efficient GPU usage)
- **Billboard Quads**: Particles always face camera for optimal appearance
- **Back-to-Front Sorting**: Correct transparency rendering
- **Particle Pool**: Efficient allocation with configurable max count (default 10,000)

### Sky Rendering
- **Procedural Sky Dome**: Gradient from horizon to zenith
- **Day-Night Cycle**: 20-minute cycle at speed 1.0 (configurable)
- **Sun Rendering**: Yellow disk moving in arc across sky
- **Moon Rendering**: White disk opposite sun
- **Atmospheric Colors**: Blue at day, dark at night, orange at sunrise/sunset
- **Infinite Distance**: View matrix without translation for sky at infinity

### Texture Quality
- **Mipmapping**: Reduces aliasing at distance, improves performance
- **Anisotropic Filtering**: Improves quality at oblique angles (up to 16x)
- **Trilinear Filtering**: Smooth mipmap transitions
- **Texture Atlas Optimization**: Efficient packing, clamp to edge prevents bleeding

### Configuration
All rendering features are configurable in `config.ini` [Rendering] section:
- Enable/disable lighting, water animation, particles, sky
- Adjust quality settings (ambient light level, water transparency, max particles)
- Performance tuning (day-night cycle speed, particle spawn rate)

## 📚 Documentation

- **[Build Guide](docs/BUILD.md)** - Detailed build instructions
- **[Architecture](docs/ARCHITECTURE.md)** - Project structure and design
- **[API Reference](docs/API.md)** - Core systems API documentation
- **[Modding Guide](docs/MODDING.md)** - Complete modding guide with tutorials
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

# Architecture Documentation

This document describes the overall architecture and design principles of the PoorCraft game engine.

## Overview

PoorCraft is a modern, cross-platform game engine built with C++17 and OpenGL 4.6. The engine follows a modular architecture with clean separation between public interfaces and implementation details.

## Project Structure

```
PoorCraft/
├── CMakeLists.txt              # Root CMake configuration
├── include/                    # Public API headers
│   └── poorcraft/              # Engine namespace
│       ├── core/               # Core systems
│       │   ├── Logger.h        # Logging system
│       │   └── Config.h        # Configuration management
│       └── platform/           # Platform abstraction
│           └── Platform.h      # OS-specific utilities
├── src/                        # Implementation files
│   ├── core/                   # Core system implementations
│   │   ├── Logger.cpp
│   │   └── Config.cpp
│   ├── platform/               # Platform-specific implementations
│   │   └── Platform.cpp
│   └── main.cpp                # Application entry point
├── libs/                       # Third-party dependencies
│   ├── glfw/                   # Window and input management
│   ├── glad/                   # OpenGL function loading
│   ├── glm/                    # OpenGL Mathematics
│   └── stb/                    # Single-file libraries
├── assets/                     # Game resources
├── shaders/                    # GLSL shader files
└── docs/                       # Documentation (API, architecture, build guides)
```

## Core Design Principles

### Modularity
The engine is designed with clear module boundaries:
- **Core systems** (logging, configuration) are independent
- **Platform abstraction** provides unified OS interface
- **Third-party dependencies** are isolated in `libs/`
- **Assets and shaders** are external to the core engine

### Cross-Platform Compatibility
- **CMake** build system supports Windows, Linux, and macOS
- **Platform abstraction** layer handles OS differences
- **Conditional compilation** for platform-specific features
- **Standard C++17** ensures portability

### Performance
- **Modern C++** features for optimal performance
- **Efficient data structures** and algorithms
- **Memory management** with RAII principles
- **Multi-threading support** where beneficial

### Maintainability
- **Clear code organization** with logical grouping
- **Comprehensive documentation** for all public APIs
- **Consistent coding standards** enforced by clang-format
- **Error handling** with graceful degradation

## System Architecture

### Core Systems

#### Logger System
**Purpose**: Centralized logging for debugging and monitoring

**Components**:
- `Logger` singleton class
- Multiple output targets (console, file)
- Configurable log levels
- Thread-safe operation
- Formatted output with timestamps

**Key Features**:
- **Log levels**: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- **Multiple outputs**: Console with colors, rotating log files
- **Thread safety**: Mutex-protected logging operations
- **Performance**: Minimal overhead when logging disabled

#### Configuration System
**Purpose**: Runtime configuration management

**Components**:
- `Config` singleton class
- INI-style configuration files
- Type-safe value access
- Change notification system

**Key Features**:
- **File format**: Simple INI-style with sections
- **Type safety**: Automatic string conversion for int/float/bool
- **Callbacks**: Notification system for configuration changes
- **Defaults**: Sensible default values for all settings

#### Window System
**Purpose**: Cross-platform window management and OpenGL context creation

**Components**:
- `Window` class wrapping GLFW
- `WindowProperties` for configuration
- `Monitor` struct for multi-monitor support
- Event callback system

**Key Features**:
- **GLFW integration**: Cross-platform window creation
- **OpenGL 4.6 Core**: Modern rendering context
- **Multi-monitor**: Query and select display devices
- **Event forwarding**: Window events to EventBus
- **VSync control**: Frame rate synchronization
- **Fullscreen support**: Windowed and fullscreen modes

#### Input System
**Purpose**: Unified input state management for keyboard, mouse, and gamepad

**Components**:
- `Input` singleton class
- State tracking arrays for all input devices
- Event-driven updates from Window callbacks
- Query interface for game logic

**Key Features**:
- **Keyboard**: Key press/release/hold detection, just-pressed/released flags
- **Mouse**: Position, delta, scroll, button states, cursor modes
- **Gamepad**: Up to 16 controllers, button and axis support
- **Frame-based updates**: Clear transient states each frame
- **Thread-safe**: Singleton pattern with safe access

#### Game Loop
**Purpose**: Fixed timestep game loop with variable rendering

**Components**:
- `GameLoop` class managing main loop
- Update and render callbacks
- Fixed timestep accumulator
- FPS limiting and measurement

**Key Features**:
- **Fixed timestep**: Consistent physics updates (default 60Hz)
- **Variable rendering**: Render as fast as possible or capped
- **Spiral prevention**: Max update iterations to avoid death spiral
- **Performance metrics**: FPS, frame time, update/render time tracking
- **Integration**: Polls events, updates Input, processes EventBus

#### Event System
**Purpose**: Engine-wide event communication using observer pattern

**Components**:
- `Event` base class with type and category system
- `EventBus` singleton for pub/sub messaging
- `EventDispatcher` for type-safe event handling
- Concrete event classes (WindowEvent, InputEvent)

**Key Features**:
- **Type hierarchy**: Base Event class with derived types
- **Categories**: Filter events by category flags
- **Pub/Sub**: Subscribe to specific event types or all events
- **Queued events**: Deferred processing for next frame
- **Thread-safe**: Mutex-protected subscription and publishing
- **Handled flag**: Prevent further processing once consumed

#### Resource Management
**Purpose**: Centralized resource loading, caching, and lifetime management

**Components**:
- `Resource` abstract base class
- `ResourceManager` singleton for caching
- `ResourceHandle` for type-safe access
- Concrete resource types (BinaryResource, etc.)

**Key Features**:
- **Caching**: Automatic cache management with path-based lookup
- **Type-safe handles**: Smart pointer-based resource access
- **Async loading**: Background loading with callbacks
- **Memory tracking**: Query total memory usage
- **Path resolution**: Base path + relative path system
- **Factory pattern**: Register custom resource types

### World & Chunk Architecture

#### Block System
**Purpose**: Define block metadata for rendering, collision, and gameplay.

**Components**:
- `BlockType` data structure (`include/poorcraft/world/BlockType.h`)
- `BlockRegistry` singleton (`include/poorcraft/world/BlockRegistry.h`)

**Key Features**:
- **ID space**: 16-bit identifiers (0 reserved for AIR) enabling up to 65,535 block types.
- **Face textures**: Per-face texture names map into the `TextureAtlas` for flexible visuals.
- **Material flags**: `isSolid`, `isOpaque`, `isTransparent`, `lightEmission`, and `hardness` prepare for collision, lighting, and mining systems.
- **Default set**: AIR, STONE, DIRT, GRASS, SAND, WATER registered during engine boot.

#### Terrain Pipeline
**Purpose**: Generate rich overworld terrain using noise-driven biomes, heightmaps, caves, ores, and structures.

**Noise Stack**:
- `TerrainGenerator::terrainNoise` (OpenSimplex FBm) sets macro elevation.
- `detailNoise` (Perlin FBm) adds small-scale perturbations.
- `BiomeMap` samples temperature, humidity, and elevation fields to select `BiomeType`.
- `caveNoise` (Ridged Simplex) punches underground cavities based on `World.cave_density`.
- `oreNoise` (Cellular) drives ore cluster attempts configurable via `World.ore_*` keys.

**Biome Selection & Blending**:
- `BiomeMap::getBiomeAt()` returns the dominant biome for a column.
- `getBlendedBiomes()` provides weights for neighboring cells; `TerrainGenerator::generateTerrain()` uses a deterministic hash to dither the top block between the two highest weights when their difference ≤ 0.15.
- `BiomeDefinition` supplies `surfaceBlock`, `subsurfaceBlock`, `undergroundBlock`, tree/grass chances, and `specialFeatures` such as `BiomeFeature::FLOWERS`.

**Heightmap Generation**:
- `TerrainGenerator::getBlendedHeight()` integrates noise output and biome heights, clamped to chunk limits.
- Bedrock occupies `y=0`; underground/subsurface/surface blocks are layered based on biome definition.

**Caves & Ores**:
- Caves carve through solid blocks when noise falls below a threshold derived from `World.cave_density`; lava fills cavities below `y=10`.
- Ore generation uses per-ore thresholds (`World.ore_coal_threshold`, etc.), frequency (`World.ore_frequency`), attempts (`World.ore_attempts_per_chunk`), and cluster size (`World.ore_cluster_size`); candidates sample `oreNoise`, then stamp clusters into stone or sandstone.

**Structure Placement**:
- `StructureGenerator` executes after terrain filling, seeded per chunk to keep deterministic distribution.
- Tree spawn chance uses biome `treeChance` scaled by `World.tree_density`; decoration chance reuses biome `grassChance` for tall grass and flowers.
- `placeFlower()` consumes the new `flower` block and texture to decorate Plains/Mountains; cacti remain desert-specific.

**Integration**:
- `TerrainGenerator::generateChunk()` orchestrates terrain, caves, ores, and structures per chunk.
- `ChunkManager` schedules generation/meshing jobs; meshes fetch UVs from `World::createBlockTextureAtlas()` (now including `flower`, `tall_grass`, ores, logs/leaves, etc.).

#### Frustum Culling
**Purpose**: Avoid rendering chunks outside the camera frustum.

**Components**:
- `Frustum` utility (`include/poorcraft/world/Frustum.h`)
- `AABB` helper for chunk bounds.

**Key Features**:
- **Plane extraction**: Gribb-Hartmann method from view-projection matrix.
- **AABB testing**: Conservative tests preventing missed visible chunks.
- **Future ready**: Sphere tests available for entity culling.

#### World Coordinator
**Purpose**: High-level façade integrating registry, texture atlas, chunk manager, and renderer.

**Components**:
- `World` class (`include/poorcraft/world/World.h`)
- Block texture atlas builder (`World::createBlockTextureAtlas()`)
- Render stats (`WorldRenderStats`)

**Key Features**:
- **Initialization flow**: Builds atlas, initializes registry, configures chunk manager with atlas pointer.
- **Runtime update**: Streams chunks based on camera position every frame.
- **Rendering**: Applies frustum culling, binds atlas texture, draws chunk VAOs, and tracks stats.
- **Extensibility**: Central place to plug in terrain generation, lighting, and gameplay.

#### Memory Management
**Purpose**: Memory allocation tracking and pooling for performance

**Components**:
- `MemoryTracker` for allocation diagnostics
- `PoolAllocator` for fixed-size object pooling
- `TypedPoolAllocator` template for type-safe pools

**Key Features**:
- **Allocation tracking**: Record all allocations with source location
- **Peak usage**: Track maximum memory usage
- **Leak detection**: Dump active allocations on shutdown
- **Object pooling**: Efficient allocation for frequently created objects
- **Thread-safe**: Mutex-protected operations

### Platform Abstraction

#### Platform Utilities
**Purpose**: Unified interface for OS-specific operations

**Components**:
- File system operations
- Timing and threading
- System information queries
- Path manipulation utilities

**Key Features**:
- **File I/O**: Binary and text file operations
- **Directory operations**: Creation, deletion, listing
- **System info**: CPU count, memory usage, platform detection
- **Path utilities**: Normalization, joining, validation

## Dependency Management

### Third-Party Libraries

#### GLFW (Window and Input)
- **Version**: Latest stable
- **Purpose**: Cross-platform window creation and input handling
- **Integration**: CMake subdirectory with custom options
- **Disabled**: Documentation, tests, examples for faster builds

#### GLAD (OpenGL Loading)
- **Purpose**: Runtime OpenGL function pointer loading
- **Configuration**: OpenGL 4.6 Core profile
- **Integration**: Static library with platform-specific linking

#### GLM (OpenGL Mathematics)
- **Purpose**: Vector, matrix, and quaternion mathematics
- **Configuration**: Header-only interface library
- **Features**: Modern C++ with constexpr support

#### stb_image (Image Loading)
- **Purpose**: Single-file image loading library
- **Configuration**: Static library with implementation guard
- **Features**: PNG, JPG, BMP, TGA support

### Dependency Integration Strategy

1. **Git submodules** for version control
2. **CMake integration** for build system consistency
3. **Isolated builds** to prevent conflicts
4. **Feature disabling** for faster compilation

## Build System Architecture

### CMake Configuration

#### Root CMakeLists.txt
- **Project setup**: Name, version, description
- **C++ standard**: C++17 with extensions disabled
- **Platform detection**: WIN32, UNIX, APPLE macros
- **Subdirectories**: libs/, src/, assets/, shaders/

#### Library CMakeLists.txt
- **Dependency order**: Correct linking dependencies
- **Feature control**: Disable unnecessary components
- **Export targets**: For main project consumption

#### Source CMakeLists.txt
- **Source discovery**: Automatic .cpp file collection
- **Executable target**: Main PoorCraft executable
- **Library linking**: All third-party dependencies
- **Platform libraries**: OS-specific linking requirements

## Coding Standards

### C++17 Features Used

- **Filesystem library** (`std::filesystem`)
- **String views** (`std::string_view`)
- **Optional types** (`std::optional`)
- **Variant types** (`std::variant`)
- **Structured bindings**
- **Constexpr functions** and variables

### Naming Conventions

- **Classes**: PascalCase (e.g., `Logger`, `Config`)
- **Functions**: camelCase (e.g., `initialize()`, `loadFromFile()`)
- **Variables**: camelCase (e.g., `logLevel`, `filePath`)
- **Constants**: SCREAMING_SNAKE_CASE (e.g., `MAX_LOG_SIZE`)
- **Namespaces**: lowercase (e.g., `poorcraft`)
- **Files**: Match class names with .h/.cpp extensions

### Code Organization

- **Header files**: Public interfaces in `include/`
- **Implementation files**: Private implementations in `src/`
- **Third-party code**: Isolated in `libs/` subdirectories
- **Tests**: Separate directory (planned for future)

## Initialization Flow

1. **Main entry point** (`main.cpp`)
2. **Logger initialization** with configuration-based log level
3. **Platform detection** and system information gathering
4. **Configuration loading** from `config.ini` file
5. **GLFW initialization** for window management
6. **Window creation** with OpenGL 4.6 Core context
7. **GLAD loading** of OpenGL function pointers
8. **Input system initialization** linked to window
9. **EventBus setup** with event forwarding from window
10. **ResourceManager initialization** with base path
11. **GameLoop creation** with update/render callbacks
12. **Main loop execution** until window closes
13. **Cleanup and shutdown** of all systems

## System Dependencies

```
Main
 ├─> Logger (independent)
 ├─> Config (independent)
 ├─> Platform (independent)
 ├─> GLFW (external)
 ├─> Window
 │    ├─> GLFW
 │    ├─> GLAD
 │    └─> Event
 ├─> EventBus
 │    └─> Event
 ├─> Input
 │    ├─> Window
 │    └─> Event
 ├─> ResourceManager
 │    ├─> Platform
 │    └─> Resource
 └─> GameLoop
      ├─> Window
      ├─> Input
      └─> EventBus
```

## Future Systems

The current architecture provides a solid foundation for additional systems:

### Planned Systems
- **Rendering Engine**: OpenGL-based rendering pipeline with shaders
- **World System**: Voxel world generation and chunk management
- **Entity System**: Entity-component architecture for game objects
- **Networking**: Multiplayer server and client
- **Audio System**: 3D spatial audio with OpenAL
- **Scripting**: Lua or similar for modding support
- **Physics**: Collision detection and response

### Extension Points
- **Plugin architecture** for third-party modules
- **Asset pipeline** for content creation tools
- **Profiling system** for performance monitoring
- **Testing framework** for automated testing

## Performance Considerations

### Memory Management
- **RAII principle** for resource ownership
- **Smart pointers** where appropriate
- **Object pooling** for frequently created objects
- **Memory arenas** for performance-critical systems

### Multi-threading
- **Thread-safe logging** with mutex protection
- **Async asset loading** for non-blocking I/O
- **Job system** for parallel task execution
- **Thread-local storage** where beneficial

### Optimization Strategies
- **Compile-time evaluation** with constexpr
- **Template metaprogramming** for zero-cost abstractions
- **Inline functions** for small utility functions
- **Link-time optimization** for better code generation

## Security Considerations

- **Input validation** for configuration files
- **Path traversal protection** in file operations
- **Buffer overflow prevention** with safe string functions
- **Error handling** without information disclosure

## Testing Strategy

### Unit Testing (Planned)
- **Test framework**: Google Test or similar
- **Coverage**: Core systems, utilities, platform abstraction
- **Mocking**: Platform-specific functions for cross-platform testing
- **CI integration**: Automated testing on all platforms

### Integration Testing (Planned)
- **Full system tests** for complete workflows
- **Performance benchmarks** for optimization validation
- **Cross-platform testing** to ensure consistency

## Documentation Strategy

- **API documentation**: Doxygen comments for all public interfaces
- **Architecture decisions**: ADR (Architecture Decision Records)
- **Build documentation**: Comprehensive platform-specific guides
- **Contributing guidelines**: Clear development workflow

## Deployment Strategy

- **Cross-platform binaries**: Single build system for all platforms
- **Package managers**: Support for vcpkg, Conan, etc.
- **Installation**: CMake install targets for system integration
- **Distribution**: Archives and installers for end users

This architecture provides a solid, extensible foundation for the PoorCraft game engine while maintaining clean separation of concerns and cross-platform compatibility.

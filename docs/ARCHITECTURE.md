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
└── docs/                       # Documentation
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
5. **Core systems verification** with comprehensive testing
6. **Engine ready** for game logic implementation

## Future Systems

The current architecture provides a solid foundation for additional systems:

### Planned Systems
- **Rendering Engine**: OpenGL-based rendering pipeline
- **World System**: Voxel world generation and management
- **Entity System**: Entity-component architecture
- **Networking**: Multiplayer server and client
- **Audio System**: 3D spatial audio
- **Input System**: Unified input handling
- **Resource Management**: Asset loading and caching
- **Scripting**: Lua or similar for modding

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

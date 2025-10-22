# PoorCraft

An open-source Minecraft clone built with Java and LWJGL, featuring multiplayer support, extensive modding capabilities, and cross-platform distribution.

## Features

### Current (Phase 1-5)
- Multi-module Gradle project structure
- Cross-platform build system (Windows, Linux, macOS)
- Logging framework (SLF4J + Log4j2)
- Configuration management (YAML-based)
- Mod API foundation
- **Networking**: Complete client-server networking with Netty
- **World System**: Authoritative server world with chunk streaming
- **Player Management**: Player sessions and entity synchronization
- **Multiplayer**: Dedicated server and client-server protocol

## Requirements

- **Java**: JDK 17 or higher
- **Gradle**: 8.5+ (included via wrapper)
- **OS**: Windows 10+, Linux (Ubuntu 20.04+), macOS 10.14+
- **GPU**: OpenGL 3.3+ capable graphics card

## Building

### Clone the Repository

```bash
git clone https://github.com/yourusername/poorcraft.git
cd poorcraft
```

### Build All Modules

```bash
./gradlew build
```

### Run the Client

```bash
./gradlew :client:run
```

### Run the Server

```bash
./gradlew :server:run
```

### Run Tests

```bash
## Multiplayer

### Running the Server

Start the dedicated server:

```bash
./gradlew :server:run
```

**Command Line Options:**
- `--port <port>`: Set server port (default: 25565)
- `--world <name>`: Set world name (default: world)

**Configuration:**
The server reads configuration from `config/server.yml`. Key settings:
- `network.serverPort`: Server port
- `network.maxPlayers`: Maximum concurrent players
- `network.enableCompression`: Enable packet compression
- `world.seed`: World seed for generation

### Connecting from Client

Run the client in multiplayer mode:

```bash
./gradlew :client:run --args="--multiplayer"
```

The client will:
1. Show a server browser (currently uses default server from config)
2. Connect to the selected server
3. Prompt for username
4. Attempt login and join the game

**Configuration:**
Client configuration in `config/client.yml`:
- `multiplayer.defaultServerAddress`: Default server to connect to
- `multiplayer.defaultServerPort`: Default server port
- `network.clientThreads`: Number of Netty client threads

### Protocol Features

- **Connection States**: HANDSHAKE → LOGIN → PLAY
- **Packet System**: Bi-directional with ID-based routing
- **Compression**: Optional Zlib compression above threshold
- **Keep-Alive**: Automatic connection health monitoring
- **Chunk Streaming**: Efficient world data transmission
- **Entity Sync**: Server-authoritative entity positions
- **Block Updates**: Real-time world state synchronization

### Troubleshooting

**Connection Issues:**
- Ensure server port is not blocked by firewall
- Check that server is running and accessible
- Verify protocol version compatibility

**Performance Issues:**
- Adjust `network.compressionThreshold` for better bandwidth usage
- Modify `world.viewDistance` for client performance
- Monitor server logs for bottlenecks

```
poorcraft/
├── common/          # Shared code (utilities, data structures, constants)
├── core/            # Game engine core (game loop, resource management)
├── client/          # Game client (rendering, UI, input)
├── server/          # Dedicated server (networking, world management)
├── modapi/          # Public API for mod developers
├── buildSrc/        # Custom Gradle plugins for distribution
├── distribution/    # Platform-specific packaging configurations
├── mods/            # Example mods and mod directory
└── docs/            # Additional documentation
```

## Module Overview

### common
Contains shared code used by both client and server:
- Configuration management
- Logging utilities
- Game constants
- Data structures
- Serialization
- **Networking**: Packet system, utilities, and registry

### core
Game engine foundation:
- Game loop with fixed timestep (Phase 2)
- Resource management (Phase 2)
- Threading and task scheduling (Phase 2)
- Engine state management (Phase 2)

### client
Game client application:
- LWJGL window and OpenGL context (Phase 2)
- Voxel rendering (Phase 3)
- UI and menus (Phase 4)
- Input handling (Phase 2)
- Audio system (Phase 8)
- **Networking**: Client connection management and packet handlers

### server
Dedicated server application:
- Netty-based networking (Phase 5)
- World loading and saving (Phase 3)
- Player management (Phase 4)
- Server console (Phase 5)
- Server tick loop (Phase 5)
- **World System**: Authoritative world with chunk management

### modapi
Public API for mod developers:
- Mod lifecycle management
- Event system
- Block/Item/Entity APIs (Phase 6)
- World generation hooks (Phase 7)

## Distribution

### Create Platform-Specific Installers

**Windows (.exe):**
```bash
./gradlew :client:windowsExe
```

**Linux (.deb):**
```bash
./gradlew :client:linuxDeb
```

**Linux (.rpm):**
```bash
./gradlew :client:linuxRpm
```

**macOS (.dmg):**
```bash
./gradlew :client:macDmg
```

**All platforms:**
```bash
./gradlew :client:distribute
```

Distribution outputs are located in `client/build/distributions/`.

## Development

### IDE Setup

**IntelliJ IDEA:**
1. Open the project root directory
2. IntelliJ will automatically detect the Gradle project
3. Wait for Gradle sync to complete
4. Run configurations will be created automatically

**Eclipse:**
1. Import as Gradle project
2. Run `./gradlew eclipse` to generate Eclipse project files

**VS Code:**
1. Install Java Extension Pack
2. Open the project root directory
3. VS Code will detect the Gradle project

### Code Style

This project uses `.editorconfig` for consistent formatting. Most IDEs support EditorConfig automatically.

**Java Conventions:**
- Classes: PascalCase
- Methods/Variables: camelCase
- Constants: UPPER_SNAKE_CASE
- Packages: lowercase
- Indentation: 4 spaces
- Line length: 120 characters max

### Running in Development

The client and server can be run directly from Gradle:

```bash
# Client (will be implemented in Phase 2)
./gradlew :client:run

# Server (will be implemented in Phase 5)
./gradlew :server:run --args="--port 25565"
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Quick Start for Contributors

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Write tests for new functionality
5. Ensure all tests pass (`./gradlew test`)
6. Commit your changes (`git commit -m 'feat: add amazing feature'`)
7. Push to your fork (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## Documentation

- [Architecture Overview](docs/ARCHITECTURE.md)
- [Build Instructions](docs/BUILD.md)
- [API Documentation](docs/API.md)
- [Modding Guide](modapi/README.md)
- [Contributing Guidelines](CONTRIBUTING.md)

### Phase 1: Foundation ✓
- Multi-module Gradle project
- Build system and distribution
- Logging and configuration
- Mod API foundation

### Phase 2: Engine Core (In Progress)
- Game loop with fixed timestep
- GLFW window creation
- OpenGL context initialization
- Basic input handling

### Phase 3: Rendering
- Chunk-based world rendering
- Block textures and models
- Camera and player controller
- Frustum culling

### Phase 4: Player System
- Player entity and movement
- 32x32 skin support
- Inventory system
- Basic UI

### Phase 5: Networking ✓
- Netty server implementation
- Client-server protocol
- Player synchronization
- World streaming

### Phase 6: Modding
- Mod loader implementation
- Event system
- Lua scripting support
- Hot-reloading

### Phase 7: World Generation
- Noise-based terrain generation
- 5 biomes
- Caves and structures
- World saving/loading

### Phase 8-14: Advanced Features
- Audio system
- Crafting and recipes
- Ray tracing support
- Steam integration
- Discord integration
- Performance optimization
- Polish and release

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **LWJGL**: Lightweight Java Game Library
- **Netty**: Asynchronous networking framework
- **Log4j**: Logging framework
- **SnakeYAML**: YAML parser
- **Gradle**: Build automation tool

## Contact

- **Issues**: [GitHub Issues](https://github.com/yourusername/poorcraft/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/poorcraft/discussions)
- **Discord**: [Join our server](https://discord.gg/yourserver)

---

**Note**: This project is in early development (Phase 1). Many features are planned but not yet implemented.

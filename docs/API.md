# API Reference

This document provides comprehensive API reference for the core systems in the PoorCraft game engine.

## Table of Contents

- [Logger API](#logger-api)
- [Config API](#config-api)
- [Platform API](#platform-api)
- [Window API](#window-api)
- [Input API](#input-api)
- [Game Loop API](#game-loop-api)
- [Event System API](#event-system-api)
- [Resource Management API](#resource-management-api)
- [Memory Management API](#memory-management-api)

## Logger API

The Logger system provides centralized logging functionality with multiple output targets and configurable log levels.

### LogLevel Enum

```cpp
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};
```

**Values**:
- `TRACE`: Detailed diagnostic information
- `DEBUG`: Debugging information
- `INFO`: General information messages
- `WARN`: Warning messages
- `ERROR`: Error conditions
- `FATAL`: Critical errors that may terminate the application

### Logger Class

Singleton class for thread-safe logging operations.

#### Static Methods

```cpp
static Logger& get_instance();
```

Returns the singleton instance of the logger.

#### Initialization

```cpp
void initialize(LogLevel log_level = LogLevel::INFO,
               bool log_to_file = false,
               const std::string& log_file_path = "poorcraft.log",
               size_t max_file_size_bytes = 5 * 1024 * 1024,
               int max_backup_files = 3);
```

**Parameters**:
- `log_level`: Minimum log level for output
- `log_to_file`: Whether to write logs to file
- `log_file_path`: Path to log file (default: "poorcraft.log")
- `max_file_size_bytes`: Maximum size of log file before rotation (0 to disable)
- `max_backup_files`: Maximum number of backup files to keep (0 to disable)

**Example**:
```cpp
poorcraft::Logger::get_instance().initialize(poorcraft::LogLevel::DEBUG, true, "debug.log", 1024*1024, 5);
```

#### Log Level Management

```cpp
void set_log_level(LogLevel level);
LogLevel get_log_level() const;
```

**Example**:
```cpp
poorcraft::Logger& logger = poorcraft::Logger::get_instance();
logger.set_log_level(poorcraft::LogLevel::WARN);
auto current_level = logger.get_log_level();
```

#### File Logging

```cpp
void set_file_logging(bool enable, const std::string& file_path = "poorcraft.log",
                     size_t max_file_size_bytes = 5 * 1024 * 1024,
                     int max_backup_files = 3);
```

**Parameters**:
- `enable`: Whether to enable file logging
- `file_path`: Path to log file
- `max_file_size_bytes`: Maximum size of log file before rotation (0 to disable)
- `max_backup_files`: Maximum number of backup files to keep (0 to disable)

**Example**:
```cpp
logger.set_file_logging(true, "engine.log", 2*1024*1024, 10);
```

#### Logging Methods

```cpp
void log(LogLevel level, const std::string& message,
         const std::string& file = "", int line = 0);
```

Generic logging method with source location information.

```cpp
void trace(const std::string& message, const std::string& file = "", int line = 0);
void debug(const std::string& message, const std::string& file = "", int line = 0);
void info(const std::string& message, const std::string& file = "", int line = 0);
void warn(const std::string& message, const std::string& file = "", int line = 0);
void error(const std::string& message, const std::string& file = "", int line = 0);
void fatal(const std::string& message, const std::string& file = "", int line = 0);
```

Level-specific logging methods.

**Examples**:
```cpp
logger.info("Engine initialized successfully");
logger.error("Failed to load texture: " + texture_path);
logger.debug("Processing " + std::to_string(vertices.size()) + " vertices");
```

#### Formatted Logging

```cpp
void logf(LogLevel level, const char* format, ...,
          const std::string& file = "", int line = 0);
void tracef(const char* format, ..., const std::string& file = "", int line = 0);
void debugf(const char* format, ..., const std::string& file = "", int line = 0);
void infof(const char* format, ..., const std::string& file = "", int line = 0);
void warnf(const char* format, ..., const std::string& file = "", int line = 0);
void errorf(const char* format, ..., const std::string& file = "", int line = 0);
void fatalf(const char* format, ..., const std::string& file = "", int line = 0);
```

Formatted logging methods using printf-style format strings.

**Examples**:
```cpp
logger.infof("Loaded %d assets in %.2f seconds", asset_count, load_time);
logger.errorf("Failed to open file '%s': %s", filename.c_str(), strerror(errno));
```

For formatted logging without arguments, use the non-variadic overloads:
```cpp
logger.infof("Hello world");  // No format arguments
logger.debugf("Simple message");  // No format arguments
```

#### Log Level Checking

```cpp
bool should_log(LogLevel level) const;
```

Checks if a log level would be output based on current minimum level.

**Example**:
```cpp
if (logger.should_log(poorcraft::LogLevel::DEBUG)) {
    // Expensive debug computation only when needed
    expensive_debug_operation();
}
```

#### Shutdown

```cpp
void shutdown();
```

Closes log files and performs cleanup.

**Example**:
```cpp
poorcraft::Logger::get_instance().shutdown();
```

### Convenience Macros

```cpp
#define PC_TRACE(msg) poorcraft::Logger::get_instance().trace(msg, __FILE__, __LINE__)
#define PC_DEBUG(msg) poorcraft::Logger::get_instance().debug(msg, __FILE__, __LINE__)
#define PC_INFO(msg)  poorcraft::Logger::get_instance().info(msg, __FILE__, __LINE__)
#define PC_WARN(msg)  poorcraft::Logger::get_instance().warn(msg, __FILE__, __LINE__)
#define PC_ERROR(msg) poorcraft::Logger::get_instance().error(msg, __FILE__, __LINE__)
#define PC_FATAL(msg) poorcraft::Logger::get_instance().fatal(msg, __FILE__, __LINE__)
```

Automatic source location macros for convenient logging.

**Examples**:
```cpp
PC_INFO("Application starting up");
PC_ERROR("Failed to initialize graphics subsystem");
PC_DEBUG("Player position: " + player_position.to_string());
```

### Formatted Logging Macros

For formatted logging, use the `PC_*F0` macros when there are no format arguments, and the `PC_*F` macros when there are format arguments:

```cpp
// For formatted logging without arguments (use F0 macros)
#define PC_TRACEF0(fmt)  poorcraft::Logger::get_instance().tracef(__FILE__, __LINE__, fmt)
#define PC_DEBUGF0(fmt)  poorcraft::Logger::get_instance().debugf(__FILE__, __LINE__, fmt)
#define PC_INFOF0(fmt)   poorcraft::Logger::get_instance().infof(__FILE__, __LINE__, fmt)
#define PC_WARNF0(fmt)   poorcraft::Logger::get_instance().warnf(__FILE__, __LINE__, fmt)
#define PC_ERRORF0(fmt)  poorcraft::Logger::get_instance().errorf(__FILE__, __LINE__, fmt)
#define PC_FATALF0(fmt)  poorcraft::Logger::get_instance().fatalf(__FILE__, __LINE__, fmt)

// For formatted logging with arguments (use F macros)
#define PC_TRACEF(fmt, ...) poorcraft::Logger::get_instance().tracef(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_DEBUGF(fmt, ...) poorcraft::Logger::get_instance().debugf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_INFOF(fmt, ...)  poorcraft::Logger::get_instance().infof(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_WARNF(fmt, ...)  poorcraft::Logger::get_instance().warnf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_ERRORF(fmt, ...) poorcraft::Logger::get_instance().errorf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_FATALF(fmt, ...) poorcraft::Logger::get_instance().fatalf(__FILE__, __LINE__, fmt, __VA_ARGS__)
```

**Examples**:
```cpp
// Formatted logging without arguments
PC_INFOF0("Hello world");
PC_DEBUGF0("Simple debug message");

// Formatted logging with arguments
PC_INFOF("Loaded %d assets in %.2f seconds", asset_count, load_time);
PC_ERRORF("Failed to open file '%s': %s", filename.c_str(), strerror(errno));
```

### Usage Examples

#### Basic Usage
```cpp
#include "poorcraft/core/Logger.h"

// Initialize logger
poorcraft::Logger::get_instance().initialize(poorcraft::LogLevel::INFO, true);

// Use logging macros
PC_INFO("Engine initialized");
PC_WARN("Configuration file not found, using defaults");
PC_ERROR("Failed to load texture");
```

#### Conditional Logging
```cpp
// Only compute expensive debug info when needed
if (PC_SHOULD_LOG(poorcraft::LogLevel::DEBUG)) {
    std::string debug_info = compute_expensive_debug_info();
    PC_DEBUG("Debug info: " + debug_info);
}
```

#### Custom Logger Instance
```cpp
poorcraft::Logger& logger = poorcraft::Logger::get_instance();
logger.set_log_level(poorcraft::LogLevel::DEBUG);
logger.info("Custom logger message");
```

## Config API

The Config system provides runtime configuration management with file I/O and type-safe value access.

### Config Class

Singleton class for configuration management.

#### Static Methods

```cpp
static Config& get_instance();
```

Returns the singleton instance of the configuration manager.

#### File Operations

```cpp
bool load_from_file(const std::string& file_path);
bool save_to_file(const std::string& file_path = "");
```

**Parameters**:
- `file_path`: Path to configuration file

**Return**: `true` if operation succeeded, `false` otherwise

**Examples**:
```cpp
poorcraft::Config& config = poorcraft::Config::get_instance();

if (!config.load_from_file("settings.ini")) {
    PC_ERROR("Failed to load configuration");
    return false;
}

if (!config.save_to_file("backup.ini")) {
    PC_WARN("Failed to save configuration backup");
}
```

#### Key Operations

```cpp
bool has(const std::string& key) const;
bool remove(const std::string& key, bool trigger_callback = true);
void clear(bool trigger_callback = false);
size_t size() const;
bool empty() const;
```

**Parameters**:
- `key`: Configuration key
- `trigger_callback`: Whether to trigger change callbacks

**Examples**:
```cpp
if (config.has("graphics.width")) {
    int width = config.get_int("graphics.width");
}

config.remove("debug.old_setting", true);
size_t entry_count = config.size();
```

#### Type-Safe Getters

```cpp
std::string get_string(const std::string& key, const std::string& default_value = "") const;
int get_int(const std::string& key, int default_value = 0) const;
float get_float(const std::string& key, float default_value = 0.0f) const;
bool get_bool(const std::string& key, bool default_value = false) const;
```

**Examples**:
```cpp
std::string player_name = config.get_string("player.name", "Steve");
int render_distance = config.get_int("graphics.render_distance", 8);
float volume = config.get_float("audio.master_volume", 1.0f);
bool vsync = config.get_bool("graphics.vsync", true);
```

#### Type-Safe Setters

```cpp
void set_string(const std::string& key, const std::string& value, bool trigger_callback = true);
void set_int(const std::string& key, int value, bool trigger_callback = true);
void set_float(const std::string& key, float value, bool trigger_callback = true);
void set_bool(const std::string& key, bool value, bool trigger_callback = true);
```

**Examples**:
```cpp
config.set_string("player.name", "Alex");
config.set_int("graphics.width", 1920);
config.set_float("audio.music_volume", 0.7f);
config.set_bool("debug.enabled", true);
```

#### Generic Access

```cpp
template<typename T>
T get(const std::string& key, const T& default_value) const;

template<typename T>
void set(const std::string& key, const T& value, bool trigger_callback = true);
```

**Examples**:
```cpp
// Generic getter with automatic type conversion
int port = config.get<int>("network.port", 25565);

// Generic setter
config.set<float>("graphics.fov", 90.0f);
```

#### Change Callbacks

```cpp
size_t register_change_callback(const std::string& key, ConfigChangeCallback callback);
bool unregister_change_callback(size_t callback_id);
```

**Types**:
```cpp
using ConfigChangeCallback = std::function<void(const std::string& key, const std::string& value)>;
```

**Examples**:
```cpp
// Register callback for specific key
auto callback_id = config.register_change_callback("graphics.width", [](const std::string& key, const std::string& value) {
    PC_INFO("Graphics width changed to: " + value);
    // Update window size
    resize_window(std::stoi(value));
});

// Register callback for all keys
auto global_callback_id = config.register_change_callback("", [](const std::string& key, const std::string& value) {
    PC_DEBUG("Configuration changed: " + key + " = " + value);
});

// Unregister callback
config.unregister_change_callback(callback_id);
```

#### Key Management

```cpp
std::vector<std::string> get_keys() const;
std::vector<std::string> get_keys_in_section(const std::string& section) const;
```

**Examples**:
```cpp
// Get all configuration keys
auto all_keys = config.get_keys();

// Get keys in specific section
auto graphics_keys = config.get_keys_in_section("graphics");
```

#### File Path Management

```cpp
void set_config_file_path(const std::string& file_path);
std::string get_config_file_path() const;
```

**Examples**:
```cpp
config.set_config_file_path("custom_settings.ini");
std::string current_path = config.get_config_file_path();
```

### Configuration Sections

#### Graphics Configuration
```cpp
struct GraphicsConfig {
    static constexpr const char* SECTION = "Graphics";
    static constexpr const char* WIDTH = "width";
    static constexpr const char* HEIGHT = "height";
    static constexpr const char* FULLSCREEN = "fullscreen";
    static constexpr const char* VSYNC = "vsync";
    static constexpr const char* FOV = "fov";
};
```

#### Audio Configuration
```cpp
struct AudioConfig {
    static constexpr const char* SECTION = "Audio";
    static constexpr const char* MASTER_VOLUME = "master_volume";
    static constexpr const char* MUSIC_VOLUME = "music_volume";
    static constexpr const char* SOUND_VOLUME = "sound_volume";
};
```

#### Controls Configuration
```cpp
struct ControlsConfig {
    static constexpr const char* SECTION = "Controls";
    static constexpr const char* MOUSE_SENSITIVITY = "mouse_sensitivity";
    static constexpr const char* INVERT_Y = "invert_y";
};
```

#### Gameplay Configuration
```cpp
struct GameplayConfig {
    static constexpr const char* SECTION = "Gameplay";
    static constexpr const char* RENDER_DISTANCE = "render_distance";
    static constexpr const char* DIFFICULTY = "difficulty";
};
```

#### Network Configuration
```cpp
struct NetworkConfig {
    static constexpr const char* SECTION = "Network";
    static constexpr const char* DEFAULT_PORT = "default_port";
    static constexpr const char* TIMEOUT = "timeout";
};
```

#### Engine Configuration
```cpp
struct EngineConfig {
    static constexpr const char* SECTION = "Engine";
    static constexpr const char* LOG_LEVEL = "log_level";
    static constexpr const char* MAX_FPS = "max_fps";
};
```

### Usage Examples

#### Basic Configuration Management
```cpp
#include "poorcraft/core/Config.h"

// Get configuration instance
poorcraft::Config& config = poorcraft::Config::get_instance();

// Load configuration file
config.load_from_file("config.ini");

// Access configuration values
int width = config.get_int(poorcraft::Config::GraphicsConfig::WIDTH, 1280);
float volume = config.get_float(poorcraft::Config::AudioConfig::MASTER_VOLUME, 1.0f);

// Modify configuration
config.set_bool(poorcraft::Config::GraphicsConfig::VSYNC, true);
config.set_int(poorcraft::Config::GraphicsConfig::WIDTH, 1920);

// Save changes
config.save_to_file();
```

#### Runtime Configuration Changes
```cpp
// Register callback for graphics changes
config.register_change_callback("graphics", [](const std::string& key, const std::string& value) {
    if (key == "graphics.width" || key == "graphics.height") {
        // Resize window
        int width = config.get_int("graphics.width");
        int height = config.get_int("graphics.height");
        resize_window(width, height);
    }
});

// Modify graphics settings (triggers callback)
config.set_int("graphics.width", 1920);
config.set_int("graphics.height", 1080);
```

## Platform API

The Platform API provides cross-platform utilities for file system operations, timing, and system information.

### FileOperationResult Enum

```cpp
enum class FileOperationResult {
    Success,
    FileNotFound,
    AccessDenied,
    PathTooLong,
    DiskFull,
    AlreadyExists,
    NotADirectory,
    IsADirectory,
    ReadOnly,
    UnknownError
};
```

### Platform Namespace

All Platform functions are static methods in the `poorcraft::Platform` namespace.

#### File System Operations

```cpp
bool file_exists(const std::string& file_path);
bool directory_exists(const std::string& dir_path);
```

**Examples**:
```cpp
if (poorcraft::Platform::file_exists("savegame.dat")) {
    // Load save game
}

if (poorcraft::Platform::directory_exists("screenshots")) {
    // Directory exists
}
```

#### File I/O

```cpp
FileOperationResult read_file_binary(const std::string& file_path, std::vector<uint8_t>& data);
FileOperationResult read_file_text(const std::string& file_path, std::string& text);
FileOperationResult write_file_binary(const std::string& file_path,
                                     const std::vector<uint8_t>& data,
                                     bool append = false);
FileOperationResult write_file_text(const std::string& file_path,
                                   const std::string& text,
                                   bool append = false);
```

**Examples**:
```cpp
// Read binary file
std::vector<uint8_t> data;
auto result = poorcraft::Platform::read_file_binary("texture.png", data);
if (result == poorcraft::Platform::FileOperationResult::Success) {
    // Process texture data
}

// Write text file
std::string content = "Player save data";
result = poorcraft::Platform::write_file_text("save.txt", content);
```

#### Directory Operations

```cpp
FileOperationResult create_directory(const std::string& dir_path, bool recursive = true);
FileOperationResult delete_path(const std::string& path, bool recursive = false);
FileOperationResult list_directory(const std::string& dir_path,
                                  std::vector<std::string>& entries,
                                  bool recursive = false);
```

**Examples**:
```cpp
// Create directory
auto result = poorcraft::Platform::create_directory("saves", true);

// List directory contents
std::vector<std::string> files;
result = poorcraft::Platform::list_directory("assets", files);

// Delete directory and contents
result = poorcraft::Platform::delete_path("temp_cache", true);
```

#### Path Information

```cpp
std::string get_executable_path();
std::string get_executable_directory();
std::string get_current_working_directory();
bool set_current_working_directory(const std::string& dir_path);
```

**Examples**:
```cpp
std::string exec_dir = poorcraft::Platform::get_executable_directory();
std::string assets_path = poorcraft::Platform::join_path(exec_dir, "assets");

poorcraft::Platform::set_current_working_directory(assets_path);
```

#### Path Utilities

```cpp
std::string normalize_path(const std::string& path);
std::string join_path(const std::string& base, const std::string& component);
std::string get_file_extension(const std::string& path);
std::string get_filename(const std::string& path);
std::string get_directory(const std::string& path);
bool is_absolute_path(const std::string& path);
std::string to_absolute_path(const std::string& relative_path);
char get_path_separator();
```

**Examples**:
```cpp
std::string normalized = poorcraft::Platform::normalize_path("../assets/./textures");
std::string full_path = poorcraft::Platform::join_path("base", "textures/stone.png");
std::string ext = poorcraft::Platform::get_file_extension("texture.png"); // "png"
std::string filename = poorcraft::Platform::get_filename("/path/to/file.txt"); // "file.txt"
```

#### System Information

```cpp
uint32_t get_cpu_count();
uint64_t get_total_memory();
uint64_t get_available_memory();
std::string get_platform_name();
bool is_windows();
bool is_linux();
bool is_macos();
```

**Examples**:
```cpp
uint32_t cores = poorcraft::Platform::get_cpu_count();
uint64_t memory_mb = poorcraft::Platform::get_total_memory() / (1024 * 1024);
std::string platform = poorcraft::Platform::get_platform_name();
```

#### Timing Functions

```cpp
std::chrono::high_resolution_clock::time_point get_time();
void sleep(uint32_t milliseconds);
void sleep(const std::chrono::milliseconds& duration);
```

**Examples**:
```cpp
auto start_time = poorcraft::Platform::get_time();
// ... do work ...
auto end_time = poorcraft::Platform::get_time();
auto duration = end_time - start_time;

poorcraft::Platform::sleep(1000); // Sleep for 1 second
```

#### Environment and System

```cpp
std::string get_environment_variable(const std::string& name);
bool set_environment_variable(const std::string& name, const std::string& value);
std::string get_home_directory();
std::string get_temp_directory();
std::string create_temp_file_path(const std::string& prefix = "tmp",
                                 const std::string& extension = "");
```

**Examples**:
```cpp
std::string home = poorcraft::Platform::get_home_directory();
std::string temp = poorcraft::Platform::get_temp_directory();
std::string temp_file = poorcraft::Platform::create_temp_file_path("poorcraft", ".log");
```

#### Command Execution

```cpp
int execute_command(const std::string& command,
                   std::vector<std::string>* output = nullptr,
                   const std::string& working_directory = "");
```

**Examples**:
```cpp
// Simple command execution
int exit_code = poorcraft::Platform::execute_command("ls -la");

// Capture output
std::vector<std::string> output;
exit_code = poorcraft::Platform::execute_command("git status", &output);
for (const auto& line : output) {
    PC_INFO("Git status: " + line);
}
```

### Usage Examples

#### File Operations
```cpp
#include "poorcraft/platform/Platform.h"

// Check if save file exists
if (poorcraft::Platform::file_exists("save.dat")) {
    // Load save game
    std::vector<uint8_t> save_data;
    auto result = poorcraft::Platform::read_file_binary("save.dat", save_data);
    if (result == poorcraft::Platform::FileOperationResult::Success) {
        // Process save data
        load_game(save_data);
    }
}

// Create screenshots directory
poorcraft::Platform::create_directory("screenshots", true);

// Save screenshot
std::string screenshot_path = poorcraft::Platform::join_path("screenshots",
    "screenshot_" + std::to_string(timestamp) + ".png");
poorcraft::Platform::write_file_binary(screenshot_path, image_data);
```

#### System Information
```cpp
// Display system information
PC_INFO("=== System Information ===");
PC_INFO("Platform: " + poorcraft::Platform::get_platform_name());
PC_INFO("CPU Cores: " + std::to_string(poorcraft::Platform::get_cpu_count()));
PC_INFO("Total Memory: " + std::to_string(poorcraft::Platform::get_total_memory() / (1024 * 1024)) + " MB");
PC_INFO("Available Memory: " + std::to_string(poorcraft::Platform::get_available_memory() / (1024 * 1024)) + " MB");
PC_INFO("Home Directory: " + poorcraft::Platform::get_home_directory());
```

#### Cross-Platform Path Handling
```cpp
// Get executable directory (works on Windows, Linux, macOS)
std::string exec_dir = poorcraft::Platform::get_executable_directory();

// Build asset paths
std::string texture_path = poorcraft::Platform::join_path(exec_dir, "assets/textures");
std::string shader_path = poorcraft::Platform::join_path(exec_dir, "assets/shaders");

// Normalize paths (resolve .. and .)
std::string normalized = poorcraft::Platform::normalize_path("../assets/./textures");

// Check if path is absolute
if (!poorcraft::Platform::is_absolute_path(config_path)) {
    config_path = poorcraft::Platform::to_absolute_path(config_path);
}
```

## Error Handling

### Logger Error Handling
- **Thread-safe** logging with mutex protection
- **Graceful degradation** if file logging fails
- **Automatic retry** for temporary failures
- **Detailed error messages** with context information

### Config Error Handling
- **Safe parsing** of malformed configuration files
- **Default values** for missing or invalid settings
- **Validation** of configuration values
- **Error logging** for debugging configuration issues

### Platform Error Handling
- **Detailed error codes** for different failure types
- **Cross-platform compatibility** for error conditions
- **Resource cleanup** on operation failures
- **Error context** preservation for debugging

## Thread Safety

### Logger Thread Safety
- **Singleton pattern** with static instance
- **Mutex protection** for all logging operations
- **Atomic operations** for log level checks
- **Thread-local storage** for performance optimization

### Config Thread Safety
- **Singleton pattern** with static instance
- **Mutex protection** for all configuration operations
- **Callback safety** with exception handling
- **Read/write locking** for concurrent access

### Platform Thread Safety
- **Stateless design** for most utility functions
- **Thread-safe system calls** where applicable
- **No global state** modification in most functions
- **Safe for concurrent use** across multiple threads

## Window API

The Window system provides cross-platform window management with GLFW integration and OpenGL context creation.

### WindowProperties Struct

```cpp
struct WindowProperties {
    std::string title = "PoorCraft";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool fullscreen = false;
    bool vsync = true;
    int monitorIndex = -1; // -1 for primary monitor
};
```

### Monitor Struct

```cpp
struct Monitor {
    int id;
    std::string name;
    int x, y;           // Position
    int width, height;  // Size
    int refreshRate;
    std::vector<VideoMode> videoModes;
};
```

### Window Class

```cpp
class Window {
public:
    explicit Window(const WindowProperties& props);
    
    // Lifecycle
    bool initialize();
    void shutdown();
    bool isOpen() const;
    
    // Event handling
    void pollEvents();
    void swapBuffers();
    void setEventCallback(std::function<void(Event&)> callback);
    
    // Getters
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    const std::string& getTitle() const;
    bool isFullscreen() const;
    bool isVSync() const;
    GLFWwindow* getNativeWindow() const;
    
    // Setters
    void setTitle(const std::string& title);
    void setSize(uint32_t width, uint32_t height);
    void setFullscreen(bool fullscreen);
    void setVSync(bool vsync);
    void setPosition(int x, int y);
    
    // Static GLFW management
    static bool initializeGLFW();
    static void terminateGLFW();
    static std::vector<Monitor> getMonitors();
    static Monitor getPrimaryMonitor();
};
```

**Example**:
```cpp
#include "poorcraft/window/Window.h"

// Initialize GLFW
if (!PoorCraft::Window::initializeGLFW()) {
    return 1;
}

// Query monitors
auto monitors = PoorCraft::Window::getMonitors();
for (const auto& monitor : monitors) {
    std::cout << monitor.name << ": " << monitor.width << "x" << monitor.height << std::endl;
}

// Create window
PoorCraft::WindowProperties props;
props.title = "My Game";
props.width = 1920;
props.height = 1080;
props.vsync = true;

PoorCraft::Window window(props);
if (!window.initialize()) {
    return 1;
}

// Set event callback
window.setEventCallback([](PoorCraft::Event& e) {
    // Handle events
});

// Main loop
while (window.isOpen()) {
    window.pollEvents();
    // Render...
    window.swapBuffers();
}

window.shutdown();
PoorCraft::Window::terminateGLFW();
```

## Input API

The Input system provides unified input state management for keyboard, mouse, and gamepad.

### Input Class

```cpp
class Input {
public:
    static Input& getInstance();
    
    // Keyboard
    bool isKeyPressed(int keyCode) const;
    bool isKeyReleased(int keyCode) const;
    bool isKeyHeld(int keyCode) const;
    bool wasKeyJustPressed(int keyCode) const;
    bool wasKeyJustReleased(int keyCode) const;
    
    // Mouse
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    bool wasMouseButtonJustPressed(int button) const;
    bool wasMouseButtonJustReleased(int button) const;
    Vec2 getMousePosition() const;
    Vec2 getMouseDelta() const;
    Vec2 getMouseScroll() const;
    void setCursorMode(CursorMode mode);
    
    // Gamepad
    bool isGamepadConnected(int gamepadId) const;
    bool isGamepadButtonPressed(int gamepadId, int button) const;
    float getGamepadAxis(int gamepadId, int axis) const;
    std::string getGamepadName(int gamepadId) const;
    
    // System
    void update();
    void onEvent(Event& event);
    void setWindow(Window* window);
};
```

**Example**:
```cpp
#include "poorcraft/input/Input.h"

// Initialize input system
PoorCraft::Input::getInstance().setWindow(&window);

// In game loop update callback
auto& input = PoorCraft::Input::getInstance();

// Keyboard input
if (input.isKeyPressed(GLFW_KEY_W)) {
    moveForward();
}
if (input.wasKeyJustPressed(GLFW_KEY_SPACE)) {
    jump();
}

// Mouse input
auto mousePos = input.getMousePosition();
auto mouseDelta = input.getMouseDelta();
rotateCamera(mouseDelta.x, mouseDelta.y);

if (input.wasMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    placeBlock();
}

// Gamepad input
if (input.isGamepadConnected(0)) {
    float leftStickX = input.getGamepadAxis(0, 0);
    float leftStickY = input.getGamepadAxis(0, 1);
    movePlayer(leftStickX, leftStickY);
}
```

## Game Loop API

The GameLoop system manages the main game loop with fixed timestep updates and variable rendering.

### GameLoop Class

```cpp
class GameLoop {
public:
    using UpdateCallback = std::function<void(float)>;
    using RenderCallback = std::function<void()>;
    
    explicit GameLoop(Window& window);
    
    // Main loop control
    void run();
    void stop();
    
    // Callbacks
    void setUpdateCallback(UpdateCallback callback);
    void setRenderCallback(RenderCallback callback);
    
    // Timing configuration
    void setFixedTimestep(float timestep);
    void setMaxFPS(int maxFPS);
    
    // Getters
    float getFPS() const;
    float getFrameTime() const;
    float getUpdateTime() const;
    float getRenderTime() const;
    bool isRunning() const;
};
```

**Example**:
```cpp
#include "poorcraft/core/GameLoop.h"

PoorCraft::GameLoop gameLoop(window);

// Set fixed timestep (60 updates per second)
gameLoop.setFixedTimestep(1.0f / 60.0f);

// Set max FPS (144 FPS cap)
gameLoop.setMaxFPS(144);

// Set update callback
gameLoop.setUpdateCallback([](float deltaTime) {
    // Update game logic with fixed timestep
    updatePhysics(deltaTime);
    updateEntities(deltaTime);
});

// Set render callback
gameLoop.setRenderCallback([]() {
    // Render scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderWorld();
    renderEntities();
});

// Run the game loop
gameLoop.run();

// Query performance metrics
float fps = gameLoop.getFPS();
float frameTime = gameLoop.getFrameTime();
```

## Event System API

The Event system provides engine-wide event communication using the observer pattern.

### Event Base Class

```cpp
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowMinimize, WindowMove,
    KeyPress, KeyRelease,
    MouseMove, MouseButtonPress, MouseButtonRelease, MouseScroll,
    GamepadButton, GamepadAxis
};

enum EventCategory {
    None = 0,
    EventCategoryWindow = 1 << 0,
    EventCategoryInput = 1 << 1,
    EventCategoryKeyboard = 1 << 2,
    EventCategoryMouse = 1 << 3,
    EventCategoryGamepad = 1 << 4
};

class Event {
public:
    virtual EventType getType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const;
    
    bool isInCategory(EventCategory category) const;
    bool isHandled() const;
    void setHandled(bool handled = true);
};
```

### EventBus Class

```cpp
class EventBus {
public:
    static EventBus& getInstance();
    
    // Subscribe to specific event type, returns subscription ID
    size_t subscribe(EventType type, EventListener listener);
    
    // Unsubscribe using subscription ID
    void unsubscribe(size_t subscriptionId);
    
    // Publish event immediately to all listeners
    void publish(Event& event);
    
    // Queue event for deferred processing
    void queueEvent(std::unique_ptr<Event> event);
    
    // Process all queued events
    void processEvents();
    
    // Clear all subscriptions and queued events
    void clear();
};
```

### Event Classes

```cpp
// Window events
class WindowCloseEvent : public Event;
class WindowResizeEvent : public Event;
class WindowFocusEvent : public Event;
class WindowMinimizeEvent : public Event;
class WindowMoveEvent : public Event;

// Input events
class KeyPressEvent : public Event;
class KeyReleaseEvent : public Event;
class MouseMoveEvent : public Event;
class MouseButtonPressEvent : public Event;
class MouseButtonReleaseEvent : public Event;
class MouseScrollEvent : public Event;
class GamepadButtonEvent : public Event;
class GamepadAxisEvent : public Event;
```

**Example**:
```cpp
#include "poorcraft/core/EventBus.h"
#include "poorcraft/events/WindowEvent.h"

// Subscribe to window close event
auto subId = PoorCraft::EventBus::getInstance().subscribe(
    PoorCraft::EventType::WindowClose,
    [](PoorCraft::Event& e) {
        std::cout << "Window closing!" << std::endl;
        // Save game, cleanup, etc.
    }
);

// Subscribe to window resize
PoorCraft::EventBus::getInstance().subscribe(
    PoorCraft::EventType::WindowResize,
    [](PoorCraft::Event& e) {
        auto& resizeEvent = static_cast<PoorCraft::WindowResizeEvent&>(e);
        std::cout << "Window resized: " << resizeEvent.getWidth() 
                  << "x" << resizeEvent.getHeight() << std::endl;
        updateViewport(resizeEvent.getWidth(), resizeEvent.getHeight());
    }
);

// Queue an event for later processing
auto event = std::make_unique<PoorCraft::KeyPressEvent>(GLFW_KEY_ESCAPE, false);
PoorCraft::EventBus::getInstance().queueEvent(std::move(event));

// Process queued events (typically called once per frame)
PoorCraft::EventBus::getInstance().processEvents();

// Unsubscribe
PoorCraft::EventBus::getInstance().unsubscribe(subId);
```

## Resource Management API

The Resource Management system provides centralized resource loading, caching, and lifetime management.

### Resource Base Class

```cpp
enum class ResourceType {
    Unknown, Texture, Shader, Model, Sound, Font, Config, Binary
};

enum class ResourceState {
    Unloaded, Loading, Loaded, Failed
};

class Resource {
public:
    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual ResourceType getType() const = 0;
    
    ResourceState getState() const;
    const std::string& getPath() const;
    size_t getSize() const;
};
```

### ResourceManager Class

```cpp
class ResourceManager {
public:
    static ResourceManager& getInstance();
    
    // Load resource (checks cache first)
    template<typename T>
    ResourceHandle<T> load(const std::string& path, const ResourceLoadParams& params = {});
    
    // Unload resource from cache
    void unload(const std::string& path);
    
    // Reload resource (force reload even if cached)
    template<typename T>
    ResourceHandle<T> reload(const std::string& path);
    
    // Get cached resource without loading
    template<typename T>
    ResourceHandle<T> get(const std::string& path);
    
    // Check if resource exists in cache
    bool exists(const std::string& path) const;
    
    // Clear all resources
    void clear();
    
    // Memory usage
    size_t getMemoryUsage() const;
    
    // Path management
    void setBasePath(const std::string& path);
    std::string resolvePath(const std::string& relativePath) const;
    
    // Async loading
    template<typename T>
    std::future<ResourceHandle<T>> loadAsync(const std::string& path, 
        std::function<void(ResourceHandle<T>)> callback = nullptr);
};
```

### ResourceHandle Template

```cpp
template<typename T>
class ResourceHandle {
public:
    T* get() const;
    T* operator->() const;
    T& operator*() const;
    bool isValid() const;
    explicit operator bool() const;
};
```

### BinaryResource Class

```cpp
class BinaryResource : public Resource {
public:
    bool load() override;
    void unload() override;
    ResourceType getType() const override;
    
    const std::vector<uint8_t>& getData() const;
    const uint8_t* getDataPtr() const;
};
```

**Example**:
```cpp
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/resource/BinaryResource.h"

// Set base path for resources
PoorCraft::ResourceManager::getInstance().setBasePath("assets/");

// Load a binary resource
auto resource = PoorCraft::ResourceManager::getInstance()
    .load<PoorCraft::BinaryResource>("data/config.dat");

if (resource.isValid()) {
    const auto& data = resource->getData();
    std::cout << "Loaded " << data.size() << " bytes" << std::endl;
}

// Async loading
auto future = PoorCraft::ResourceManager::getInstance()
    .loadAsync<PoorCraft::BinaryResource>("data/large_file.dat",
        [](auto handle) {
            if (handle.isValid()) {
                std::cout << "Async load complete!" << std::endl;
            }
        });

// Query memory usage
size_t memoryUsage = PoorCraft::ResourceManager::getInstance().getMemoryUsage();
std::cout << "Resource memory: " << memoryUsage / 1024 << " KB" << std::endl;

// Clear all resources
PoorCraft::ResourceManager::getInstance().clear();
```

## Memory Management API

The Memory Management system provides allocation tracking and object pooling for performance.

### MemoryTracker Class

```cpp
class MemoryTracker {
public:
    static MemoryTracker& getInstance();
    
    // Record allocations
    void recordAllocation(void* ptr, size_t size, const char* file, int line);
    void recordDeallocation(void* ptr);
    
    // Statistics
    size_t getTotalAllocated() const;
    size_t getAllocationCount() const;
    size_t getPeakMemoryUsage() const;
    
    // Debugging
    void dumpAllocations() const;
    void reset();
};
```

### PoolAllocator Class

```cpp
class PoolAllocator {
public:
    PoolAllocator(size_t elementSize, size_t capacity);
    
    // Allocation
    void* allocate();
    void deallocate(void* ptr);
    void reset();
    
    // Getters
    size_t getElementSize() const;
    size_t getCapacity() const;
    size_t getUsedCount() const;
    size_t getFreeCount() const;
};

// Template version for type-safe allocation
template<typename T>
class TypedPoolAllocator {
public:
    explicit TypedPoolAllocator(size_t capacity);
    
    template<typename... Args>
    T* construct(Args&&... args);
    
    void destroy(T* ptr);
    void reset();
};
```

**Example**:
```cpp
#include "poorcraft/memory/MemoryTracker.h"
#include "poorcraft/memory/PoolAllocator.h"

// Memory tracking
auto& tracker = PoorCraft::MemoryTracker::getInstance();
tracker.recordAllocation(ptr, size, __FILE__, __LINE__);
// ... later ...
tracker.recordDeallocation(ptr);

// Query statistics
std::cout << "Total allocated: " << tracker.getTotalAllocated() << " bytes" << std::endl;
std::cout << "Peak usage: " << tracker.getPeakMemoryUsage() << " bytes" << std::endl;

// Dump all active allocations (for leak detection)
tracker.dumpAllocations();

// Object pooling
PoorCraft::TypedPoolAllocator<MyEntity> entityPool(1000);

// Allocate from pool
MyEntity* entity = entityPool.construct(arg1, arg2);

// Use entity...

// Return to pool
entityPool.destroy(entity);

// Reset pool (returns all objects to free list)
entityPool.reset();
```

This API reference covers all core systems available in the current version of PoorCraft. Additional systems will be documented as they are implemented.

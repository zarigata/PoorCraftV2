# API Reference

This document provides comprehensive API reference for the core systems in the PoorCraft game engine.

## Table of Contents

- [Logger API](#logger-api)
- [Config API](#config-api)
- [Platform API](#platform-api)

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

This API reference covers the core systems available in the current version of PoorCraft. Additional systems will be documented as they are implemented.

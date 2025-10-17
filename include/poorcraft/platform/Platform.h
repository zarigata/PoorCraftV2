#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace poorcraft {

// Platform detection macros
#if defined(_WIN32)
    #define PC_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PC_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define PC_PLATFORM_MACOS
#else
    #error "Unsupported platform"
#endif

/**
 * @brief Platform abstraction utilities namespace
 *
 * The Platform namespace provides a unified interface for platform-specific
 * operations including file system access, timing, system information, and
 * path manipulation. All functions are static and thread-safe where applicable.
 */
namespace Platform {

/**
 * @brief File system operation results
 */
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

/**
 * @brief Convert FileOperationResult to string
 */
std::string file_operation_result_to_string(FileOperationResult result);

/**
 * @brief Check if a file exists
 * @param file_path Path to the file to check
 * @return True if the file exists, false otherwise
 */
bool file_exists(const std::string& file_path);

/**
 * @brief Check if a directory exists
 * @param dir_path Path to the directory to check
 * @return True if the directory exists, false otherwise
 */
bool directory_exists(const std::string& dir_path);

/**
 * @brief Read a file as binary data
 * @param file_path Path to the file to read
 * @param data Vector to store the file data
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult read_file_binary(const std::string& file_path, std::vector<uint8_t>& data);

/**
 * @brief Read a file as text
 * @param file_path Path to the file to read
 * @param text String to store the file contents
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult read_file_text(const std::string& file_path, std::string& text);

/**
 * @brief Write binary data to a file
 * @param file_path Path to the file to write
 * @param data Binary data to write
 * @param append Whether to append to existing file or overwrite
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult write_file_binary(const std::string& file_path,
                                     const std::vector<uint8_t>& data,
                                     bool append = false);

/**
 * @brief Write text to a file
 * @param file_path Path to the file to write
 * @param text Text to write
 * @param append Whether to append to existing file or overwrite
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult write_file_text(const std::string& file_path,
                                   const std::string& text,
                                   bool append = false);

/**
 * @brief Create a directory
 * @param dir_path Path to the directory to create
 * @param recursive Whether to create parent directories if they don't exist
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult create_directory(const std::string& dir_path, bool recursive = true);

/**
 * @brief Delete a file or directory
 * @param path Path to delete
 * @param recursive Whether to delete directory contents recursively
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult delete_path(const std::string& path, bool recursive = false);

/**
 * @brief Get the size of a file in bytes
 * @param file_path Path to the file
 * @param size Reference to store the file size
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult get_file_size(const std::string& file_path, uint64_t& size);

/**
 * @brief List files and directories in a directory
 * @param dir_path Path to the directory to list
 * @param entries Vector to store the directory entries
 * @param recursive Whether to list contents recursively
 * @return FileOperationResult indicating success or failure
 */
FileOperationResult list_directory(const std::string& dir_path,
                                  std::vector<std::string>& entries,
                                  bool recursive = false);

/**
 * @brief Get the path to the current executable
 * @return Full path to the executable, or empty string on error
 */
std::string get_executable_path();

/**
 * @brief Get the directory containing the executable
 * @return Directory path, or empty string on error
 */
std::string get_executable_directory();

/**
 * @brief Get the current working directory
 * @return Current working directory path
 */
std::string get_current_working_directory();

/**
 * @brief Set the current working directory
 * @param dir_path New working directory path
 * @return True if successful, false otherwise
 */
bool set_current_working_directory(const std::string& dir_path);

/**
 * @brief Get high-resolution time point
 * @return Current time point
 */
std::chrono::high_resolution_clock::time_point get_time();

/**
 * @brief Sleep for a specified duration
 * @param milliseconds Duration to sleep in milliseconds
 */
void sleep(uint32_t milliseconds);

/**
 * @brief Sleep for a specified duration
 * @param duration Duration to sleep
 */
void sleep(const std::chrono::milliseconds& duration);

/**
 * @brief Get the number of CPU cores available
 * @return Number of CPU cores, or 1 if unable to determine
 */
uint32_t get_cpu_count();

/**
 * @brief Get the total system memory in bytes
 * @return Total memory in bytes, or 0 if unable to determine
 */
uint64_t get_total_memory();

/**
 * @brief Get the available system memory in bytes
 * @return Available memory in bytes, or 0 if unable to determine
 */
uint64_t get_available_memory();

/**
 * @brief Get the platform name as a string
 * @return Platform name ("Windows", "Linux", or "macOS")
 */
std::string get_platform_name();

/**
 * @brief Check if the platform is Windows
 * @return True if running on Windows
 */
bool is_windows();

/**
 * @brief Check if the platform is Linux
 * @return True if running on Linux
 */
bool is_linux();

/**
 * @brief Check if the platform is macOS
 * @return True if running on macOS
 */
bool is_macos();

/**
 * @brief Normalize a file path (resolve .. and . components)
 * @param path Path to normalize
 * @return Normalized path
 */
std::string normalize_path(const std::string& path);

/**
 * @brief Join two path components
 * @param base Base path
 * @param component Component to append
 * @return Joined path with correct separator
 */
std::string join_path(const std::string& base, const std::string& component);

/**
 * @brief Get the file extension from a path
 * @param path File path
 * @return File extension (without the dot), or empty string if none
 */
std::string get_file_extension(const std::string& path);

/**
 * @brief Get the filename from a path (without directory)
 * @param path File path
 * @return Filename, or empty string if path is directory
 */
std::string get_filename(const std::string& path);

/**
 * @brief Get the directory part of a path
 * @param path File path
 * @return Directory path, or empty string if no directory
 */
std::string get_directory(const std::string& path);

/**
 * @brief Check if a path is absolute
 * @param path Path to check
 * @return True if path is absolute
 */
bool is_absolute_path(const std::string& path);

/**
 * @brief Convert a relative path to absolute
 * @param relative_path Relative path
 * @return Absolute path, or empty string on error
 */
std::string to_absolute_path(const std::string& relative_path);

/**
 * @brief Get the path separator for the current platform
 * @return Path separator character
 */
char get_path_separator();

/**
 * @brief Get the last modification time of a file
 * @param file_path Path to the file
 * @return Modification time point, or time_point{} on error
 */
std::chrono::system_clock::time_point get_file_modification_time(const std::string& file_path);

/**
 * @brief Get environment variable value
 * @param name Environment variable name
 * @return Variable value, or empty string if not found
 */
std::string get_environment_variable(const std::string& name);

/**
 * @brief Set environment variable
 * @param name Environment variable name
 * @param value Variable value
 * @return True if successful, false otherwise
 */
bool set_environment_variable(const std::string& name, const std::string& value);

/**
 * @brief Execute a system command
 * @param command Command to execute
 * @param output Vector to store command output
 * @param working_directory Working directory for the command
 * @return Exit code of the command, or -1 on error
 */
int execute_command(const std::string& command,
                   std::vector<std::string>* output = nullptr,
                   const std::string& working_directory = "");

/**
 * @brief Get the user's home directory
 * @return Home directory path, or empty string on error
 */
std::string get_home_directory();

/**
 * @brief Get a temporary directory path
 * @return Temporary directory path, or empty string on error
 */
std::string get_temp_directory();

/**
 * @brief Create a unique temporary file path
 * @param prefix Prefix for the temporary file
 * @param extension File extension (with dot)
 * @return Unique temporary file path, or empty string on error
 */
std::string create_temp_file_path(const std::string& prefix = "tmp",
                                 const std::string& extension = "");

/**
 * @brief Get system information as a formatted string
 * @return String containing platform, CPU, and memory information
 */
std::string get_system_info();

} // namespace Platform

} // namespace poorcraft

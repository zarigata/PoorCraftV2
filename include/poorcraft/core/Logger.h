#pragma once

#include <string>
#include <mutex>
#include <memory>
#include <fstream>
#include <sstream>

namespace poorcraft {

/**
 * @brief Log levels for filtering and formatting log messages
 */
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

/**
 * @brief Convert LogLevel to string representation
 * @param level The log level to convert
 * @return String representation of the log level
 */
std::string log_level_to_string(LogLevel level);

/**
 * @brief Convert string to LogLevel
 * @param str String representation of log level
 * @return Corresponding LogLevel or INFO if not recognized
 */
LogLevel string_to_log_level(const std::string& str);

/**
 * @brief Singleton logger class for thread-safe logging throughout the engine
 *
 * The Logger class provides a centralized logging system with multiple output
 * targets (console, file), configurable log levels, and thread-safe operation.
 * It supports both formatted string logging and stream-style logging.
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the logger
     * @return Reference to the logger instance
     */
    static Logger& get_instance();

    /**
     * @brief Initialize the logger with configuration
     * @param log_level Minimum log level to output
     * @param log_to_file Whether to log to file in addition to console
     * @param log_file_path Path to the log file (optional, defaults to "poorcraft.log")
     * @param max_file_size_bytes Maximum size of log file before rotation (0 to disable)
     * @param max_backup_files Maximum number of backup files to keep (0 to disable)
     */
    void initialize(LogLevel log_level = LogLevel::INFO,
                   bool log_to_file = false,
                   const std::string& log_file_path = "poorcraft.log",
                   size_t max_file_size_bytes = 5 * 1024 * 1024,  // 5MB default
                   int max_backup_files = 3);

    /**
     * @brief Shutdown the logger and close log files
     */
    void shutdown();

    /**
     * @brief Set the minimum log level for output
     * @param level New minimum log level
     */
    void set_log_level(LogLevel level);

    /**
     * @brief Get the current minimum log level
     * @return Current log level
     */
    LogLevel get_log_level() const;

    /**
     * @brief Enable or disable file logging
     * @param enable Whether to log to file
     * @param file_path Path to the log file
     * @param max_file_size_bytes Maximum size of log file before rotation (0 to disable)
     * @param max_backup_files Maximum number of backup files to keep (0 to disable)
     */
    void set_file_logging(bool enable, const std::string& file_path = "poorcraft.log",
                         size_t max_file_size_bytes = 5 * 1024 * 1024,
                         int max_backup_files = 3);

    /**
     * @brief Log a message with specified level
     * @param level Log level
     * @param message Message to log
     * @param file Source file name (auto-filled by macros)
     * @param line Source line number (auto-filled by macros)
     */
    void log(LogLevel level, const std::string& message,
             const std::string& file = "", int line = 0);

    /**
     * @brief Log a formatted message with specified level
     * @param level Log level
     * @param file Source file name (auto-filled by macros)
     * @param line Source line number (auto-filled by macros)
     * @param format Format string (printf-style)
     * @param ... Format arguments
     */
    void logf(LogLevel level, const std::string& file, int line, const char* format, ...);

    /**
     * @brief Check if a log level would be output
     * @param level Log level to check
     * @return True if the level would be logged
     */
    bool should_log(LogLevel level) const;

    // Convenience logging methods

    /**
     * @brief Log a trace message
     */
    void trace(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log a debug message
     */
    void debug(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log an info message
     */
    void info(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log a warning message
     */
    void warn(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log an error message
     */
    void error(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log a fatal message
     */
    void fatal(const std::string& message, const std::string& file = "", int line = 0);

    /**
     * @brief Log a formatted trace message
     */
    void tracef(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted debug message
     */
    void debugf(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted info message
     */
    void infof(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted warning message
     */
    void warnf(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted error message
     */
    void errorf(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted fatal message
     */
    void fatalf(const std::string& file, int line, const char* format, ...);

    /**
     * @brief Log a formatted trace message (no arguments)
     */
    void tracef(const std::string& file, int line, const char* format);

    /**
     * @brief Log a formatted debug message (no arguments)
     */
    void debugf(const std::string& file, int line, const char* format);

    /**
     * @brief Log a formatted info message (no arguments)
     */
    void infof(const std::string& file, int line, const char* format);

    /**
     * @brief Log a formatted warning message (no arguments)
     */
    void warnf(const std::string& file, int line, const char* format);

    /**
     * @brief Log a formatted error message (no arguments)
     */
    void errorf(const std::string& file, int line, const char* format);

    /**
     * @brief Log a formatted fatal message (no arguments)
     */
    void fatalf(const std::string& file, int line, const char* format);

private:
    Logger() = default;
    ~Logger() = default;

    // Disable copy and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Internal logging implementation
     */
    void log_internal(LogLevel level, const std::string& message,
                     const std::string& file, int line);

    /**
     * @brief Format current time as string
     */
    std::string format_timestamp();

    /**
     * @brief Write message to console with appropriate coloring
     */
    void write_to_console(LogLevel level, const std::string& formatted_message);

    /**
     * @brief Write message to log file
     */
    void write_to_file(const std::string& formatted_message);

    /**
     * @brief Perform log file rotation if needed
     */
    void perform_rotation();

    /**
     * @brief Format log message with level, timestamp, file, and line info
     */
    std::string format_message(LogLevel level, const std::string& message,
                              const std::string& file, int line);

private:
    LogLevel min_log_level_ = LogLevel::INFO;
    bool log_to_file_ = false;
    std::string log_file_path_;
    std::ofstream log_file_;
    mutable std::mutex mutex_;

    // Log rotation configuration
    size_t max_file_size_bytes_ = 5 * 1024 * 1024;  // 5MB default
    int max_backup_files_ = 3;
};

// Convenience macros for logging
#define PC_TRACE(msg) poorcraft::Logger::get_instance().trace(msg, __FILE__, __LINE__)
#define PC_DEBUG(msg) poorcraft::Logger::get_instance().debug(msg, __FILE__, __LINE__)
#define PC_INFO(msg)  poorcraft::Logger::get_instance().info(msg, __FILE__, __LINE__)
#define PC_WARN(msg)  poorcraft::Logger::get_instance().warn(msg, __FILE__, __LINE__)
#define PC_ERROR(msg) poorcraft::Logger::get_instance().error(msg, __FILE__, __LINE__)
#define PC_FATAL(msg) poorcraft::Logger::get_instance().fatal(msg, __FILE__, __LINE__)

// Formatted logging macros - use PC_*F0 for no arguments, PC_*F for formatted output
#define PC_TRACEF0(fmt)  poorcraft::Logger::get_instance().tracef(__FILE__, __LINE__, fmt)
#define PC_DEBUGF0(fmt)  poorcraft::Logger::get_instance().debugf(__FILE__, __LINE__, fmt)
#define PC_INFOF0(fmt)   poorcraft::Logger::get_instance().infof(__FILE__, __LINE__, fmt)
#define PC_WARNF0(fmt)   poorcraft::Logger::get_instance().warnf(__FILE__, __LINE__, fmt)
#define PC_ERRORF0(fmt)  poorcraft::Logger::get_instance().errorf(__FILE__, __LINE__, fmt)
#define PC_FATALF0(fmt)  poorcraft::Logger::get_instance().fatalf(__FILE__, __LINE__, fmt)

#define PC_TRACEF(fmt, ...) poorcraft::Logger::get_instance().tracef(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_DEBUGF(fmt, ...) poorcraft::Logger::get_instance().debugf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_INFOF(fmt, ...)  poorcraft::Logger::get_instance().infof(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_WARNF(fmt, ...)  poorcraft::Logger::get_instance().warnf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_ERRORF(fmt, ...) poorcraft::Logger::get_instance().errorf(__FILE__, __LINE__, fmt, __VA_ARGS__)
#define PC_FATALF(fmt, ...) poorcraft::Logger::get_instance().fatalf(__FILE__, __LINE__, fmt, __VA_ARGS__)

} // namespace poorcraft

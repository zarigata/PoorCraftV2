#include "poorcraft/core/Logger.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdarg>
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define ISATTY _isatty
    #define FILENO _fileno
#else
    #include <unistd.h>
    #define ISATTY isatty
    #define FILENO fileno
#endif

namespace poorcraft {

std::string log_level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

LogLevel string_to_log_level(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);

    if (upper_str == "TRACE") return LogLevel::TRACE;
    if (upper_str == "DEBUG") return LogLevel::DEBUG;
    if (upper_str == "INFO")  return LogLevel::INFO;
    if (upper_str == "WARN")  return LogLevel::WARN;
    if (upper_str == "ERROR") return LogLevel::ERROR;
    if (upper_str == "FATAL") return LogLevel::FATAL;

    return LogLevel::INFO; // Default fallback
}

Logger& Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(LogLevel log_level, bool log_to_file, const std::string& log_file_path,
                       size_t max_file_size_bytes, int max_backup_files) {
    std::lock_guard<std::mutex> lock(mutex_);

    min_log_level_ = log_level;
    max_file_size_bytes_ = max_file_size_bytes;
    max_backup_files_ = max_backup_files;

    if (log_to_file) {
        log_file_path_ = log_file_path;
        log_file_.open(log_file_path_, std::ios::out | std::ios::app);
        log_to_file_ = log_file_.is_open();
        if (!log_to_file_) {
            std::cerr << "Failed to open log file: " << log_file_path_ << std::endl;
        }
    } else {
        log_to_file_ = false;
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::set_log_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_log_level_ = level;
}

LogLevel Logger::get_log_level() const {
    return min_log_level_;
}

void Logger::set_file_logging(bool enable, const std::string& file_path,
                             size_t max_file_size_bytes, int max_backup_files) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (enable && !log_to_file_) {
        log_file_path_ = file_path;
        max_file_size_bytes_ = max_file_size_bytes;
        max_backup_files_ = max_backup_files;
        log_file_.open(log_file_path_, std::ios::out | std::ios::app);
        log_to_file_ = log_file_.is_open();
        if (!log_to_file_) {
            std::cerr << "Failed to open log file: " << log_file_path_ << std::endl;
        }
    } else if (!enable && log_to_file_) {
        log_file_.close();
        log_to_file_ = false;
    }
}

void Logger::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    if (!should_log(level)) {
        return;
    }

    log_internal(level, message, file, line);
}

void Logger::logf(LogLevel level, const std::string& file, int line, const char* format, ...) {
    if (!should_log(level)) {
        return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    log_internal(level, buffer, file, line);
}

bool Logger::should_log(LogLevel level) const {
    return static_cast<int>(level) >= static_cast<int>(min_log_level_);
}

void Logger::trace(const std::string& message, const std::string& file, int line) {
    log(LogLevel::TRACE, message, file, line);
}

void Logger::debug(const std::string& message, const std::string& file, int line) {
    log(LogLevel::DEBUG, message, file, line);
}

void Logger::info(const std::string& message, const std::string& file, int line) {
    log(LogLevel::INFO, message, file, line);
}

void Logger::warn(const std::string& message, const std::string& file, int line) {
    log(LogLevel::WARN, message, file, line);
}

void Logger::error(const std::string& message, const std::string& file, int line) {
    log(LogLevel::ERROR, message, file, line);
}

void Logger::fatal(const std::string& message, const std::string& file, int line) {
    log(LogLevel::FATAL, message, file, line);
}

void Logger::tracef(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    trace(buffer, file, line);
}

void Logger::debugf(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    debug(buffer, file, line);
}

void Logger::infof(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    info(buffer, file, line);
}

void Logger::warnf(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    warn(buffer, file, line);
}

void Logger::errorf(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    error(buffer, file, line);
}

void Logger::errorf(const std::string& file, int line, const char* format) {
    error(format, file, line);
}

void Logger::fatalf(const std::string& file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    fatal(buffer, file, line);
}

void Logger::tracef(const std::string& file, int line, const char* format) {
    trace(format, file, line);
}

void Logger::debugf(const std::string& file, int line, const char* format) {
    debug(format, file, line);
}

void Logger::infof(const std::string& file, int line, const char* format) {
    info(format, file, line);
}

void Logger::warnf(const std::string& file, int line, const char* format) {
    warn(format, file, line);
}

void Logger::fatalf(const std::string& file, int line, const char* format) {
    fatal(format, file, line);
}

void Logger::log_internal(LogLevel level, const std::string& message,
                         const std::string& file, int line) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string formatted_message = format_message(level, message, file, line);

    write_to_console(level, formatted_message);

    if (log_to_file_) {
        write_to_file(formatted_message);
    }
}

std::string Logger::format_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03lld",
             tm.tm_hour, tm.tm_min, tm.tm_sec,
             static_cast<long long>(ms.count()));

    return buffer;
}

void Logger::write_to_console(LogLevel level, const std::string& formatted_message) {
    // Check if console supports colors
    static bool color_supported = ISATTY(FILENO(stdout)) != 0;

    if (color_supported) {
#ifdef _WIN32
        // Windows console coloring
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // White

        switch (level) {
            case LogLevel::TRACE:
                color = FOREGROUND_INTENSITY; // Bright white
                break;
            case LogLevel::DEBUG:
                color = FOREGROUND_BLUE | FOREGROUND_GREEN; // Cyan
                break;
            case LogLevel::INFO:
                color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Bright green
                break;
            case LogLevel::WARN:
                color = FOREGROUND_RED | FOREGROUND_GREEN; // Yellow
                break;
            case LogLevel::ERROR:
            case LogLevel::FATAL:
                color = FOREGROUND_RED | FOREGROUND_INTENSITY; // Bright red
                break;
        }

        SetConsoleTextAttribute(hConsole, color);
        std::cout << formatted_message << std::endl;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
        // ANSI color codes for Unix-like systems
        const char* color_code = "";
        switch (level) {
            case LogLevel::TRACE:
                color_code = "\033[37m"; // White
                break;
            case LogLevel::DEBUG:
                color_code = "\033[36m"; // Cyan
                break;
            case LogLevel::INFO:
                color_code = "\033[32m"; // Green
                break;
            case LogLevel::WARN:
                color_code = "\033[33m"; // Yellow
                break;
            case LogLevel::ERROR:
            case LogLevel::FATAL:
                color_code = "\033[31m"; // Red
                break;
        }

        std::cout << color_code << formatted_message << "\033[0m" << std::endl;
#endif
    } else {
        std::cout << formatted_message << std::endl;
    }

    // Always flush for immediate output
    std::cout.flush();

    // For fatal errors, also write to stderr
    if (level == LogLevel::FATAL) {
        std::cerr << formatted_message << std::endl;
        std::cerr.flush();
    }
}

void Logger::write_to_file(const std::string& formatted_message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!log_file_.is_open()) {
        return;
    }

    // Check if rotation is needed
    if (max_file_size_bytes_ > 0) {
        try {
            std::filesystem::path file_path(log_file_path_);
            if (std::filesystem::exists(file_path)) {
                size_t current_size = std::filesystem::file_size(file_path);
                if (current_size + formatted_message.size() + 1 >= max_file_size_bytes_) {
                    perform_rotation();
                }
            }
        } catch (const std::exception& e) {
            // Log error but continue writing
            std::cerr << "Error checking file size for rotation: " << e.what() << std::endl;
        }
    }

    if (log_file_.is_open()) {
        log_file_ << formatted_message << std::endl;
        log_file_.flush();
    }
}

void Logger::perform_rotation() {
    // Caller must hold mutex_
    
    if (max_file_size_bytes_ == 0 || max_backup_files_ <= 0) {
        return; // Rotation disabled
    }

    // Close current file
    log_file_.close();

    try {
        // Delete oldest backup if it exists
        std::string oldest_backup = log_file_path_ + "." + std::to_string(max_backup_files_);
        if (std::filesystem::exists(oldest_backup)) {
            std::filesystem::remove(oldest_backup);
        }

        // Rename backup files: .3 -> .4, .2 -> .3, .1 -> .2
        for (int i = max_backup_files_ - 1; i >= 1; --i) {
            std::string old_name = log_file_path_ + "." + std::to_string(i);
            std::string new_name = log_file_path_ + "." + std::to_string(i + 1);
            if (std::filesystem::exists(old_name)) {
                std::filesystem::rename(old_name, new_name);
            }
        }

        // Rename current file to .1
        std::string backup_name = log_file_path_ + ".1";
        if (std::filesystem::exists(log_file_path_)) {
            std::filesystem::rename(log_file_path_, backup_name);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during log rotation: " << e.what() << std::endl;
    }

    // Open new log file
    log_file_.open(log_file_path_, std::ios::out | std::ios::trunc);
    if (!log_file_.is_open()) {
        std::cerr << "Failed to reopen log file after rotation: " << log_file_path_ << std::endl;
        log_to_file_ = false;
    }
}

std::string Logger::format_message(LogLevel level, const std::string& message,
                                  const std::string& file, int line) {
    std::string location = "";
    if (!file.empty() && line > 0) {
        // Extract filename from full path
        size_t last_slash = file.find_last_of("/\\");
        std::string filename = (last_slash != std::string::npos) ? file.substr(last_slash + 1) : file;
        location = filename + ":" + std::to_string(line);
    }

    return "[" + format_timestamp() + "] [" + log_level_to_string(level) + "] " +
           (location.empty() ? "" : location + " - ") + message;
}

} // namespace poorcraft

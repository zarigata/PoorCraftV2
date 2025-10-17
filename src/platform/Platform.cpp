#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"

#include <vector>
#include <cerrno>
#include <iostream>
#include <filesystem>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #include <process.h>
    #define getcwd _getcwd
    #define chdir _chdir
    #define access _access
    #define F_OK 0
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/sysinfo.h>
    #include <sys/wait.h>
    #include <dirent.h>
    #include <errno.h>
    #include <limits.h>
#endif

#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

namespace poorcraft {
namespace Platform {

std::string file_operation_result_to_string(FileOperationResult result) {
    switch (result) {
        case FileOperationResult::Success: return "Success";
        case FileOperationResult::FileNotFound: return "FileNotFound";
        case FileOperationResult::AccessDenied: return "AccessDenied";
        case FileOperationResult::PathTooLong: return "PathTooLong";
        case FileOperationResult::DiskFull: return "DiskFull";
        case FileOperationResult::AlreadyExists: return "AlreadyExists";
        case FileOperationResult::NotADirectory: return "NotADirectory";
        case FileOperationResult::IsADirectory: return "IsADirectory";
        case FileOperationResult::ReadOnly: return "ReadOnly";
        case FileOperationResult::UnknownError: return "UnknownError";
        default: return "Unknown";
    }
}

bool file_exists(const std::string& file_path) {
    try {
        return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
    } catch (const std::exception& e) {
        PC_DEBUG("File existence check failed for " + file_path + ": " + e.what());
        return false;
    }
}

bool directory_exists(const std::string& dir_path) {
    try {
        return std::filesystem::exists(dir_path) && std::filesystem::is_directory(dir_path);
    } catch (const std::exception& e) {
        PC_DEBUG("Directory existence check failed for " + dir_path + ": " + e.what());
        return false;
    }
}

FileOperationResult read_file_binary(const std::string& file_path, std::vector<uint8_t>& data) {
    try {
        if (!std::filesystem::exists(file_path)) {
            return FileOperationResult::FileNotFound;
        }

        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return FileOperationResult::AccessDenied;
        }

        std::filesystem::path path(file_path);
        size_t file_size = std::filesystem::file_size(path);

        if (file_size > data.max_size()) {
            return FileOperationResult::PathTooLong;
        }

        data.resize(file_size);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(data.data()), file_size);
        file.close();

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Binary file read failed for " + file_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult read_file_text(const std::string& file_path, std::string& text) {
    try {
        if (!std::filesystem::exists(file_path)) {
            return FileOperationResult::FileNotFound;
        }

        std::ifstream file(file_path);
        if (!file.is_open()) {
            return FileOperationResult::AccessDenied;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        text = buffer.str();
        file.close();

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Text file read failed for " + file_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult write_file_binary(const std::string& file_path,
                                     const std::vector<uint8_t>& data,
                                     bool append) {
    try {
        std::ios::openmode mode = std::ios::binary;
        if (append) {
            mode |= std::ios::app;
        } else {
            mode |= std::ios::out;
        }

        std::ofstream file(file_path, mode);
        if (!file.is_open()) {
            return FileOperationResult::AccessDenied;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Binary file write failed for " + file_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult write_file_text(const std::string& file_path,
                                   const std::string& text,
                                   bool append) {
    try {
        std::ios::openmode mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }

        std::ofstream file(file_path, mode);
        if (!file.is_open()) {
            return FileOperationResult::AccessDenied;
        }

        file << text;
        file.close();

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Text file write failed for " + file_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult create_directory(const std::string& dir_path, bool recursive) {
    try {
        if (std::filesystem::exists(dir_path)) {
            if (std::filesystem::is_directory(dir_path)) {
                return FileOperationResult::AlreadyExists;
            } else {
                return FileOperationResult::AlreadyExists; // File with same name exists
            }
        }

        std::filesystem::create_directories(dir_path);
        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Directory creation failed for " + dir_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult delete_path(const std::string& path, bool recursive) {
    try {
        if (!std::filesystem::exists(path)) {
            return FileOperationResult::FileNotFound;
        }

        if (recursive && std::filesystem::is_directory(path)) {
            std::filesystem::remove_all(path);
        } else {
            std::filesystem::remove(path);
        }

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Path deletion failed for " + path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult get_file_size(const std::string& file_path, uint64_t& size) {
    try {
        if (!std::filesystem::exists(file_path)) {
            return FileOperationResult::FileNotFound;
        }

        size = std::filesystem::file_size(file_path);
        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("File size check failed for " + file_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

FileOperationResult list_directory(const std::string& dir_path,
                                  std::vector<std::string>& entries,
                                  bool recursive) {
    try {
        if (!std::filesystem::exists(dir_path)) {
            return FileOperationResult::FileNotFound;
        }

        if (!std::filesystem::is_directory(dir_path)) {
            return FileOperationResult::NotADirectory;
        }

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
                entries.push_back(entry.path().string());
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                entries.push_back(entry.path().string());
            }
        }

        return FileOperationResult::Success;
    } catch (const std::exception& e) {
        PC_DEBUG("Directory listing failed for " + dir_path + ": " + e.what());
        return FileOperationResult::UnknownError;
    }
}

std::string get_executable_path() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return "";
    }
    return std::string(buffer, length);
#elif __linux__
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length == -1) {
        return "";
    }
    buffer[length] = '\0';
    return buffer;
#elif __APPLE__
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) != 0) {
        return "";
    }
    return buffer;
#else
    return "";
#endif
}

std::string get_executable_directory() {
    std::string exec_path = get_executable_path();
    if (exec_path.empty()) {
        return "";
    }

    size_t last_separator = exec_path.find_last_of("/\\");
    if (last_separator == std::string::npos) {
        return "";
    }

    return exec_path.substr(0, last_separator);
}

std::string get_current_working_directory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (_getcwd(buffer, MAX_PATH) == nullptr) {
        return "";
    }
    return buffer;
#else
    // For POSIX systems, use dynamic allocation to handle paths longer than PATH_MAX
    std::vector<char> buffer(PATH_MAX);
    char* result = getcwd(buffer.data(), buffer.size());
    if (result == nullptr) {
        // If PATH_MAX is not enough, try with a larger buffer
        if (errno == ERANGE) {
            buffer.resize(buffer.size() * 2);
            result = getcwd(buffer.data(), buffer.size());
            if (result == nullptr) {
                return "";
            }
        } else {
            return "";
        }
    }
    return buffer.data();
#endif
}

bool set_current_working_directory(const std::string& dir_path) {
    return chdir(dir_path.c_str()) == 0;
}

std::chrono::high_resolution_clock::time_point get_time() {
    return std::chrono::high_resolution_clock::now();
}

void sleep(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void sleep(const std::chrono::milliseconds& duration) {
    std::this_thread::sleep_for(duration);
}

uint32_t get_cpu_count() {
    return std::thread::hardware_concurrency();
}

uint64_t get_total_memory() {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return status.ullTotalPhys;
    }
    return 0;
#elif __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;
    }
    return 0;
#elif __APPLE__
    // macOS doesn't have a simple way to get total memory
    // This is a simplified approach
    return 0;
#else
    return 0;
#endif
}

uint64_t get_available_memory() {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return status.ullAvailPhys;
    }
    return 0;
#elif __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.freeram * info.mem_unit;
    }
    return 0;
#elif __APPLE__
    return 0;
#else
    return 0;
#endif
}

std::string get_platform_name() {
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#elif __APPLE__
    return "macOS";
#else
    return "Unknown";
#endif
}

bool is_windows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool is_linux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

bool is_macos() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

std::string normalize_path(const std::string& path) {
    try {
        std::filesystem::path p(path);
        return p.lexically_normal().string();
    } catch (const std::exception& e) {
        PC_DEBUG("Path normalization failed for " + path + ": " + e.what());
        return path;
    }
}

std::string join_path(const std::string& base, const std::string& component) {
    try {
        std::filesystem::path p1(base);
        std::filesystem::path p2(component);
        return (p1 / p2).string();
    } catch (const std::exception& e) {
        PC_DEBUG("Path join failed for " + base + " + " + component + ": " + e.what());
        return base + get_path_separator() + component;
    }
}

std::string get_file_extension(const std::string& path) {
    try {
        return std::filesystem::path(path).extension().string();
    } catch (const std::exception& e) {
        PC_DEBUG("Extension extraction failed for " + path + ": " + e.what());
        return "";
    }
}

std::string get_filename(const std::string& path) {
    try {
        return std::filesystem::path(path).filename().string();
    } catch (const std::exception& e) {
        PC_DEBUG("Filename extraction failed for " + path + ": " + e.what());
        return "";
    }
}

std::string get_directory(const std::string& path) {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            return p.parent_path().string();
        }
        return "";
    } catch (const std::exception& e) {
        PC_DEBUG("Directory extraction failed for " + path + ": " + e.what());
        return "";
    }
}

bool is_absolute_path(const std::string& path) {
    try {
        return std::filesystem::path(path).is_absolute();
    } catch (const std::exception& e) {
        PC_DEBUG("Absolute path check failed for " + path + ": " + e.what());
        return false;
    }
}

std::string to_absolute_path(const std::string& relative_path) {
    try {
        return std::filesystem::absolute(relative_path).string();
    } catch (const std::exception& e) {
        PC_DEBUG("Absolute path conversion failed for " + relative_path + ": " + e.what());
        return "";
    }
}

char get_path_separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::chrono::system_clock::time_point get_file_modification_time(const std::string& file_path) {
    try {
        auto file_time = std::filesystem::last_write_time(file_path);
        return std::chrono::system_clock::time_point(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                file_time.time_since_epoch()
            )
        );
    } catch (const std::exception& e) {
        PC_DEBUG("File modification time check failed for " + file_path + ": " + e.what());
        return std::chrono::system_clock::time_point{};
    }
}

std::string get_environment_variable(const std::string& name) {
#ifdef _WIN32
    char* value;
    size_t size;
    if (_dupenv_s(&value, &size, name.c_str()) == 0 && value != nullptr) {
        std::string result(value);
        free(value);
        return result;
    }
    return "";
#else
    const char* value = getenv(name.c_str());
    return value ? value : "";
#endif
}

bool set_environment_variable(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

int execute_command(const std::string& command, std::vector<std::string>* output,
                   const std::string& working_directory) {
#ifdef _WIN32
    // Simplified Windows implementation
    // For a full implementation, would need to use CreateProcess
    PC_WARN("Command execution not fully implemented on Windows");
    return -1;
#else
    // Unix implementation using popen
    // RAII guard to restore working directory
    struct CwdGuard {
        std::string original_cwd;
        bool active = false;
        
        ~CwdGuard() {
            if (active && !original_cwd.empty()) {
                if (!set_current_working_directory(original_cwd)) {
                    // Log warning but do not throw
                    std::cerr << "Warning: Failed to restore working directory to: " << original_cwd << std::endl;
                }
            }
        }
    };
    
    CwdGuard cwd_guard;

    if (!working_directory.empty()) {
        // Save current working directory
        cwd_guard.original_cwd = get_current_working_directory();
        if (cwd_guard.original_cwd.empty()) {
            PC_ERROR("Failed to get current working directory");
            return -1;
        }

        if (chdir(working_directory.c_str()) != 0) {
            PC_ERROR("Failed to change working directory to: " + working_directory);
            return -1;
        }
        cwd_guard.active = true;
    }

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        PC_ERROR("Failed to execute command: " + command);
        return -1;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\n\r") + 1); // Trim newlines
        if (output) {
            output->push_back(line);
        }
    }

    int status = pclose(pipe);
    return WEXITSTATUS(status);
#endif
}

std::string get_home_directory() {
    const char* home = getenv("HOME");
#ifdef _WIN32
    if (!home) {
        home = getenv("USERPROFILE");
    }
#endif
    return home ? home : "";
}

std::string get_temp_directory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetTempPathA(MAX_PATH, buffer);
    if (length == 0 || length > MAX_PATH) {
        return "";
    }
    return std::string(buffer, length);
#else
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) {
        tmpdir = "/tmp";
    }
    return tmpdir;
#endif
}

std::string create_temp_file_path(const std::string& prefix, const std::string& extension) {
    std::string temp_dir = get_temp_directory();
    if (temp_dir.empty()) {
        return "";
    }

    std::string filename = prefix + "_XXXXXX";
    if (!extension.empty()) {
        filename += extension;
    }

    std::string full_path = join_path(temp_dir, filename);

#ifdef _WIN32
    char* temp_name = _mktemp(const_cast<char*>(full_path.c_str()));
    return temp_name ? temp_name : "";
#else
    char* temp_name = mktemp(const_cast<char*>(full_path.c_str()));
    return temp_name ? temp_name : "";
#endif
}

std::string get_system_info() {
    std::stringstream ss;
    ss << "Platform: " << get_platform_name() << "\n";
    ss << "CPU Cores: " << get_cpu_count() << "\n";
    ss << "Total Memory: " << (get_total_memory() / (1024 * 1024)) << " MB\n";
    ss << "Available Memory: " << (get_available_memory() / (1024 * 1024)) << " MB\n";
    return ss.str();
}

} // namespace Platform
} // namespace poorcraft

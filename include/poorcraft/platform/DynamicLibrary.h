#pragma once

#include <string>
#include <stdexcept>

namespace PoorCraft {

/**
 * @brief Cross-platform dynamic library loading wrapper
 * 
 * Provides RAII wrapper for loading shared libraries (.dll/.so/.dylib)
 * and resolving symbols. Supports Windows (LoadLibrary) and POSIX (dlopen).
 */
class DynamicLibrary {
public:
    /**
     * @brief Construct empty library (not loaded)
     */
    DynamicLibrary();

    /**
     * @brief Construct and load library
     * @param path Path to library file
     * @param lazy Use lazy symbol resolution (RTLD_LAZY on POSIX)
     */
    explicit DynamicLibrary(const std::string& path, bool lazy = true);

    /**
     * @brief Destructor - unloads library if loaded
     */
    ~DynamicLibrary();

    // Non-copyable
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    // Movable
    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;

    /**
     * @brief Load library from path
     * @param path Path to library file
     * @param lazy Use lazy symbol resolution
     * @throws std::runtime_error on load failure
     */
    void load(const std::string& path, bool lazy = true);

    /**
     * @brief Unload library
     */
    void unload();

    /**
     * @brief Check if library is loaded
     */
    bool isLoaded() const;

    /**
     * @brief Get symbol from library (throws on failure)
     * @tparam T Function pointer type
     * @param symbolName Symbol name to resolve
     * @return Function pointer
     * @throws std::runtime_error if symbol not found
     */
    template<typename T>
    T getSymbol(const char* symbolName) const {
        void* symbol = getSymbolInternal(symbolName);
        return reinterpret_cast<T>(symbol);
    }

    /**
     * @brief Try to get symbol (returns nullptr on failure)
     * @tparam T Function pointer type
     * @param symbolName Symbol name to resolve
     * @return Function pointer or nullptr
     */
    template<typename T>
    T tryGetSymbol(const char* symbolName) const {
        try {
            return getSymbol<T>(symbolName);
        } catch (const std::runtime_error&) {
            return nullptr;
        }
    }

    /**
     * @brief Get platform-specific library extension
     * @return ".dll" on Windows, ".dylib" on macOS, ".so" on Linux
     */
    static std::string getLibraryExtension();

    /**
     * @brief Add platform-specific prefix/suffix to library name
     * @param baseName Base library name (e.g., "mylib")
     * @return Decorated name (e.g., "libmylib.so" on Linux)
     */
    static std::string decorateLibraryName(const std::string& baseName);

    /**
     * @brief Get library path
     */
    const std::string& getPath() const { return m_Path; }

private:
    void* getSymbolInternal(const char* symbolName) const;

    void* m_Handle;
    std::string m_Path;
};

} // namespace PoorCraft

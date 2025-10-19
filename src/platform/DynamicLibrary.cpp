#include "poorcraft/platform/DynamicLibrary.h"
#include "poorcraft/core/Logger.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace PoorCraft {

DynamicLibrary::DynamicLibrary()
    : m_Handle(nullptr)
    , m_Path("")
{
}

DynamicLibrary::DynamicLibrary(const std::string& path, bool lazy)
    : m_Handle(nullptr)
    , m_Path("")
{
    load(path, lazy);
}

DynamicLibrary::~DynamicLibrary() {
    unload();
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
    : m_Handle(other.m_Handle)
    , m_Path(std::move(other.m_Path))
{
    other.m_Handle = nullptr;
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this != &other) {
        unload();
        m_Handle = other.m_Handle;
        m_Path = std::move(other.m_Path);
        other.m_Handle = nullptr;
    }
    return *this;
}

void DynamicLibrary::load(const std::string& path, bool lazy) {
    if (m_Handle) {
        unload();
    }

    m_Path = path;

#ifdef _WIN32
    // Convert UTF-8 to UTF-16 for Windows
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    if (wideSize == 0) {
        throw std::runtime_error("Failed to convert path to UTF-16: " + path);
    }

    std::wstring widePath(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &widePath[0], wideSize);

    m_Handle = LoadLibraryW(widePath.c_str());
    if (!m_Handle) {
        DWORD error = GetLastError();
        LPWSTR messageBuffer = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&messageBuffer,
            0,
            nullptr
        );

        std::string errorMsg = "Failed to load library: " + path;
        if (messageBuffer) {
            // Convert error message to UTF-8
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, messageBuffer, -1, nullptr, 0, nullptr, nullptr);
            if (utf8Size > 0) {
                std::string utf8Error(utf8Size, 0);
                WideCharToMultiByte(CP_UTF8, 0, messageBuffer, -1, &utf8Error[0], utf8Size, nullptr, nullptr);
                errorMsg += " - " + utf8Error;
            }
            LocalFree(messageBuffer);
        }

        throw std::runtime_error(errorMsg);
    }
#else
    // POSIX (Linux/macOS)
    int flags = (lazy ? RTLD_LAZY : RTLD_NOW) | RTLD_LOCAL;
    m_Handle = dlopen(path.c_str(), flags);
    if (!m_Handle) {
        const char* error = dlerror();
        std::string errorMsg = "Failed to load library: " + path;
        if (error) {
            errorMsg += " - " + std::string(error);
        }
        throw std::runtime_error(errorMsg);
    }
#endif

    PC_INFO("Loaded dynamic library: {}", path);
}

void DynamicLibrary::unload() {
    if (!m_Handle) {
        return;
    }

#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(m_Handle));
#else
    dlclose(m_Handle);
#endif

    PC_INFO("Unloaded dynamic library: {}", m_Path);
    m_Handle = nullptr;
    m_Path.clear();
}

bool DynamicLibrary::isLoaded() const {
    return m_Handle != nullptr;
}

void* DynamicLibrary::getSymbolInternal(const char* symbolName) const {
    if (!m_Handle) {
        throw std::runtime_error("Cannot get symbol from unloaded library");
    }

#ifdef _WIN32
    void* symbol = GetProcAddress(static_cast<HMODULE>(m_Handle), symbolName);
    if (!symbol) {
        throw std::runtime_error("Symbol not found: " + std::string(symbolName));
    }
#else
    // Clear any previous errors
    dlerror();
    void* symbol = dlsym(m_Handle, symbolName);
    const char* error = dlerror();
    if (error) {
        throw std::runtime_error("Symbol not found: " + std::string(symbolName) + " - " + error);
    }
#endif

    PC_DEBUG("Resolved symbol: {}", symbolName);
    return symbol;
}

template<typename T>
T DynamicLibrary::getSymbol(const char* symbolName) const {
    void* symbol = getSymbolInternal(symbolName);
    return reinterpret_cast<T>(symbol);
}

template<typename T>
T DynamicLibrary::tryGetSymbol(const char* symbolName) const {
    try {
        return getSymbol<T>(symbolName);
    } catch (const std::runtime_error&) {
        return nullptr;
    }
}

std::string DynamicLibrary::getLibraryExtension() {
#ifdef _WIN32
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

std::string DynamicLibrary::decorateLibraryName(const std::string& baseName) {
#ifdef _WIN32
    return baseName + ".dll";
#else
    return "lib" + baseName + getLibraryExtension();
#endif
}

// Explicit template instantiations for common function pointer types
template void* DynamicLibrary::getSymbol<void*>(const char*) const;
template void* DynamicLibrary::tryGetSymbol<void*>(const char*) const;

} // namespace PoorCraft

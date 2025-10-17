#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"

namespace PoorCraft {

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::unload(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Cache.find(path);
    if (it != m_Cache.end()) {
        it->second->unload();
        m_Cache.erase(it);
        Logger::getInstance().log(LogLevel::INFO, "ResourceManager", 
            "Unloaded resource: " + path);
    }
}

bool ResourceManager::exists(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Cache.find(path) != m_Cache.end();
}

void ResourceManager::clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    for (auto& [path, resource] : m_Cache) {
        resource->unload();
    }
    
    m_Cache.clear();
    Logger::getInstance().log(LogLevel::INFO, "ResourceManager", "Cleared all resources");
}

size_t ResourceManager::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    size_t total = 0;
    for (const auto& [path, resource] : m_Cache) {
        total += resource->getSize();
    }
    
    return total;
}

void ResourceManager::setBasePath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_BasePath = Platform::normalize_path(path);
    Logger::getInstance().log(LogLevel::INFO, "ResourceManager", 
        "Base path set to: " + m_BasePath);
}

std::string ResourceManager::resolvePath(const std::string& relativePath) const {
    if (m_BasePath.empty()) {
        return relativePath;
    }
    return Platform::join_path(m_BasePath, relativePath);
}

} // namespace PoorCraft

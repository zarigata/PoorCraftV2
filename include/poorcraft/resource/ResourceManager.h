#pragma once

#include "poorcraft/resource/Resource.h"
#include <unordered_map>
#include <functional>
#include <mutex>
#include <future>

namespace PoorCraft {

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

    // Factory registration
    template<typename T>
    void registerFactory(ResourceType type, std::function<std::shared_ptr<T>()> factory);

    // Async loading
    template<typename T>
    std::future<ResourceHandle<T>> loadAsync(const std::string& path, 
        std::function<void(ResourceHandle<T>)> callback = nullptr);

    // Non-copyable, non-movable
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    mutable std::mutex m_Mutex;
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_Cache;
    std::unordered_map<ResourceType, std::function<std::shared_ptr<Resource>()>> m_Factories;
    std::string m_BasePath;
};

// Template implementations
template<typename T>
ResourceHandle<T> ResourceManager::load(const std::string& path, const ResourceLoadParams& params) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    // Check cache first
    auto it = m_Cache.find(path);
    if (it != m_Cache.end()) {
        auto resource = std::dynamic_pointer_cast<T>(it->second);
        if (resource && resource->getState() == ResourceState::Loaded) {
            return ResourceHandle<T>(resource);
        }
    }
    
    // Create new resource
    auto resource = std::make_shared<T>(resolvePath(path));
    resource->setState(ResourceState::Loading);
    
    if (!resource->load()) {
        resource->setState(ResourceState::Failed);
        return ResourceHandle<T>();
    }
    
    resource->setState(ResourceState::Loaded);
    m_Cache[path] = resource;
    
    return ResourceHandle<T>(resource);
}

template<typename T>
ResourceHandle<T> ResourceManager::reload(const std::string& path) {
    unload(path);
    return load<T>(path);
}

template<typename T>
ResourceHandle<T> ResourceManager::get(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    auto it = m_Cache.find(path);
    if (it != m_Cache.end()) {
        auto resource = std::dynamic_pointer_cast<T>(it->second);
        if (resource) {
            return ResourceHandle<T>(resource);
        }
    }
    
    return ResourceHandle<T>();
}

template<typename T>
void ResourceManager::registerFactory(ResourceType type, std::function<std::shared_ptr<T>()> factory) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Factories[type] = [factory]() -> std::shared_ptr<Resource> {
        return std::static_pointer_cast<Resource>(factory());
    };
}

template<typename T>
std::future<ResourceHandle<T>> ResourceManager::loadAsync(const std::string& path,
    std::function<void(ResourceHandle<T>)> callback) {
    return std::async(std::launch::async, [this, path, callback]() {
        auto handle = load<T>(path);
        if (callback) {
            callback(handle);
        }
        return handle;
    });
}

} // namespace PoorCraft

#pragma once

#include <string>
#include <memory>

namespace PoorCraft {

enum class ResourceType {
    Unknown,
    Texture,
    Shader,
    Model,
    Sound,
    Font,
    Config,
    Binary
};

enum class ResourceState {
    Unloaded,
    Loading,
    Loaded,
    Failed
};

// Abstract base class for all resources
class Resource {
public:
    explicit Resource(const std::string& path);
    virtual ~Resource() = default;

    // Pure virtual methods
    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual ResourceType getType() const = 0;

    // Getters
    ResourceState getState() const { return m_State; }
    const std::string& getPath() const { return m_Path; }
    size_t getSize() const { return m_Size; }

    // State management helpers
    void setState(ResourceState state) { m_State = state; }
    void setSize(size_t size) { m_Size = size; }

    // Non-copyable but movable
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

protected:
    std::string m_Path;
    ResourceState m_State = ResourceState::Unloaded;
    size_t m_Size = 0;
};

// Resource handle for type-safe access
template<typename T>
class ResourceHandle {
public:
    ResourceHandle() = default;
    explicit ResourceHandle(std::shared_ptr<T> resource)
        : m_Resource(std::move(resource)) {}

    T* get() const { return m_Resource.get(); }
    T* operator->() const { return m_Resource.get(); }
    T& operator*() const { return *m_Resource; }

    bool isValid() const {
        return m_Resource && m_Resource->getState() == ResourceState::Loaded;
    }

    explicit operator bool() const { return isValid(); }

    std::shared_ptr<T> getSharedPtr() const { return m_Resource; }

private:
    std::shared_ptr<T> m_Resource;
};

// Resource load parameters
struct ResourceLoadParams {
    bool async = false;
    int priority = 0;
};

} // namespace PoorCraft

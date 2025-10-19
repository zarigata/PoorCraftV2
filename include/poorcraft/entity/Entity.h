#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/Component.h"

namespace PoorCraft {

using EntityID = std::uint64_t;
using ComponentDeleter = void(*)(void*);
using ComponentPtr = std::unique_ptr<void, ComponentDeleter>;

class Entity {
public:
    Entity(EntityID id, std::string name);
    ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    template <typename T, typename... Args>
    T& addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "Component must derive from Component base");
        const std::type_index typeIndex(typeid(T));
        auto it = m_Components.find(typeIndex);
        if (it != m_Components.end()) {
            return *static_cast<T*>(it->second.get());
        }

        T* instance = new T(std::forward<Args>(args)...);
        ComponentPtr holder(instance, [](void* ptr) {
            delete static_cast<T*>(ptr);
        });
        T& reference = *instance;
        m_Components.emplace(typeIndex, std::move(holder));
        PC_TRACE("Entity '" + m_Name + "' added component: " + std::string(typeid(T).name()));
        return reference;
    }

    template <typename T>
    T* getComponent() {
        const std::type_index typeIndex(typeid(T));
        auto it = m_Components.find(typeIndex);
        if (it == m_Components.end()) {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    const T* getComponent() const {
        const std::type_index typeIndex(typeid(T));
        auto it = m_Components.find(typeIndex);
        if (it == m_Components.end()) {
            return nullptr;
        }
        return static_cast<const T*>(it->second.get());
    }

    template <typename T>
    bool hasComponent() const {
        return m_Components.find(std::type_index(typeid(T))) != m_Components.end();
    }

    template <typename T>
    void removeComponent() {
        const std::type_index typeIndex(typeid(T));
        if (m_Components.erase(typeIndex) > 0) {
            PC_TRACE("Entity '" + m_Name + "' removed component: " + std::string(typeid(T).name()));
        }
    }

    EntityID getId() const;
    const std::string& getName() const;
    bool isActive() const;
    void setActive(bool active);

private:
    EntityID m_Id;
    std::string m_Name;
    bool m_Active = true;
    std::unordered_map<std::type_index, ComponentPtr> m_Components;
};

} // namespace PoorCraft

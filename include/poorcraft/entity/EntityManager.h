#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "poorcraft/entity/Entity.h"

namespace PoorCraft {

class EntityManager {
public:
    static EntityManager& getInstance();

    Entity& createEntity(const std::string& name);
    void destroyEntity(EntityID id);

    Entity* getEntity(EntityID id);
    const Entity* getEntity(EntityID id) const;

    const std::unordered_map<EntityID, std::unique_ptr<Entity>>& getAllEntities() const;

    template <typename T>
    std::vector<Entity*> getEntitiesWith();

    template <typename T>
    std::vector<const Entity*> getEntitiesWith() const;

    void clear();
    std::size_t getEntityCount() const;

private:
    EntityManager() = default;

    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    mutable std::mutex m_Mutex;
    std::unordered_map<EntityID, std::unique_ptr<Entity>> m_Entities;
    EntityID m_NextEntityId = 1;
};

template <typename T>
std::vector<Entity*> EntityManager::getEntitiesWith() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<Entity*> result;
    result.reserve(m_Entities.size());

    for (auto& [id, entity] : m_Entities) {
        if (entity && entity->hasComponent<T>()) {
            result.push_back(entity.get());
        }
    }

    return result;
}

template <typename T>
std::vector<const Entity*> EntityManager::getEntitiesWith() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    std::vector<const Entity*> result;
    result.reserve(m_Entities.size());

    for (const auto& [id, entity] : m_Entities) {
        if (entity && entity->hasComponent<T>()) {
            result.push_back(entity.get());
        }
    }

    return result;
}

} // namespace PoorCraft

#include "poorcraft/entity/EntityManager.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/modding/ModEvents.h"
#include "poorcraft/entity/components/Transform.h"

namespace PoorCraft {

EntityManager& EntityManager::getInstance() {
    static EntityManager instance;
    return instance;
}

Entity& EntityManager::createEntity(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    EntityID id = m_NextEntityId++;
    auto entity = std::make_unique<Entity>(id, name);
    Entity& reference = *entity;
    m_Entities.emplace(id, std::move(entity));

    PC_INFO("Entity created (id=" + std::to_string(id) + ", name=" + name + ")");
    
    // Publish EntitySpawnedEvent after entity is fully initialized
    // Note: Position will be (0,0,0) if Transform hasn't been added yet
    glm::vec3 position(0.0f);
    if (reference.hasComponent<Transform>()) {
        position = reference.getComponent<Transform>()->position;
    }
    EntitySpawnedEvent spawnEvent(id, name, position, 0); // spawnedBy=0 for system-spawned
    EventBus::getInstance().publish(spawnEvent);
    
    return reference;
}

void EntityManager::destroyEntity(EntityID id) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Entities.find(id);
    if (it != m_Entities.end()) {
        // Publish EntityDestroyedEvent before entity is removed
        EntityDestroyedEvent destroyEvent(id, "Destroyed");
        EventBus::getInstance().publish(destroyEvent);
        
        PC_INFO("Entity destroyed (id=" + std::to_string(id) + ")");
        m_Entities.erase(it);
    }
}

Entity* EntityManager::getEntity(EntityID id) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Entities.find(id);
    if (it == m_Entities.end()) {
        return nullptr;
    }
    return it->second.get();
}

const Entity* EntityManager::getEntity(EntityID id) const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Entities.find(id);
    if (it == m_Entities.end()) {
        return nullptr;
    }
    return it->second.get();
}

const std::unordered_map<EntityID, std::unique_ptr<Entity>>& EntityManager::getAllEntities() const {
    return m_Entities;
}

void EntityManager::clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Entities.clear();
    m_NextEntityId = 1;
    PC_INFO("EntityManager cleared");
}

std::size_t EntityManager::getEntityCount() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Entities.size();
}

} // namespace PoorCraft

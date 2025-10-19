#include "poorcraft/modding/ModAPI.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/BlockType.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"

#include <cstring>
#include <unordered_map>

namespace PoorCraft {

// Storage for event subscriptions (for cleanup)
static std::unordered_map<uint32_t, size_t> g_EventSubscriptions;
static uint32_t g_NextSubscriptionId = 1;

// Block API implementations
static uint16_t API_RegisterBlock(const char* name, bool isSolid, bool isOpaque, bool isTransparent,
                                   const char* textureName, float hardness) {
    try {
        BlockType block;
        block.setName(name)
             .setSolid(isSolid)
             .setOpaque(isOpaque)
             .setTransparent(isTransparent)
             .setTextureAllFaces(textureName)
             .setHardness(hardness);
        
        uint16_t id = BlockRegistry::getInstance().registerBlock(block);
        PC_INFO("Mod registered block: {} (ID: {})", name, id);
        return id;
    } catch (const std::exception& e) {
        PC_ERROR("Failed to register block '{}': {}", name, e.what());
        return 0;
    }
}

static uint16_t API_GetBlockID(const char* name) {
    try {
        return BlockRegistry::getInstance().getBlockID(name);
    } catch (...) {
        return 0;
    }
}

static bool API_GetBlockName(uint16_t id, char* outBuffer, size_t bufferSize) {
    try {
        const BlockType& block = BlockRegistry::getInstance().getBlock(id);
        if (outBuffer && bufferSize > 0) {
            std::strncpy(outBuffer, block.name.c_str(), bufferSize - 1);
            outBuffer[bufferSize - 1] = '\0';
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

static bool API_SetBlockAt(int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t blockId) {
    try {
        // TODO: Implement via ChunkManager when available
        // For now, just log the attempt
        PC_DEBUG("Mod attempted to set block {} at ({}, {}, {})", blockId, worldX, worldY, worldZ);
        return true;
    } catch (...) {
        return false;
    }
}

static uint16_t API_GetBlockAt(int32_t worldX, int32_t worldY, int32_t worldZ) {
    try {
        // TODO: Implement via ChunkManager when available
        PC_DEBUG("Mod queried block at ({}, {}, {})", worldX, worldY, worldZ);
        return 0; // AIR
    } catch (...) {
        return 0;
    }
}

// Entity API implementations
static uint32_t API_SpawnEntity(const char* name, float x, float y, float z) {
    try {
        Entity& entity = EntityManager::getInstance().createEntity(name);
        
        // Add Transform component with position
        if (!entity.hasComponent<Transform>()) {
            auto& transform = entity.addComponent<Transform>();
            transform.position = glm::vec3(x, y, z);
        }
        
        PC_INFO("Mod spawned entity: {} (ID: {}) at ({}, {}, {})", name, entity.getId(), x, y, z);
        return static_cast<uint32_t>(entity.getId());
    } catch (const std::exception& e) {
        PC_ERROR("Failed to spawn entity '{}': {}", name, e.what());
        return 0;
    }
}

static bool API_DestroyEntity(uint32_t entityId) {
    try {
        EntityManager::getInstance().destroyEntity(static_cast<EntityID>(entityId));
        PC_DEBUG("Mod destroyed entity: {}", entityId);
        return true;
    } catch (...) {
        return false;
    }
}

static bool API_GetEntityPosition(uint32_t entityId, float* outX, float* outY, float* outZ) {
    try {
        Entity* entity = EntityManager::getInstance().getEntity(static_cast<EntityID>(entityId));
        if (!entity) return false;
        
        Transform* transform = entity->getComponent<Transform>();
        if (!transform) return false;
        
        if (outX) *outX = transform->position.x;
        if (outY) *outY = transform->position.y;
        if (outZ) *outZ = transform->position.z;
        return true;
    } catch (...) {
        return false;
    }
}

static bool API_SetEntityPosition(uint32_t entityId, float x, float y, float z) {
    try {
        Entity* entity = EntityManager::getInstance().getEntity(static_cast<EntityID>(entityId));
        if (!entity) return false;
        
        Transform* transform = entity->getComponent<Transform>();
        if (!transform) return false;
        
        transform->position = glm::vec3(x, y, z);
        return true;
    } catch (...) {
        return false;
    }
}

// Event API implementations
static uint32_t API_SubscribeEvent(uint32_t eventType, EventCallback callback, void* userData) {
    try {
        // Create wrapper lambda that calls C callback
        auto listener = [callback, userData](Event& event) {
            if (callback) {
                callback(&event, userData);
            }
        };
        
        size_t busSubscriptionId = EventBus::getInstance().subscribe(static_cast<EventType>(eventType), listener);
        
        uint32_t modSubscriptionId = g_NextSubscriptionId++;
        g_EventSubscriptions[modSubscriptionId] = busSubscriptionId;
        
        PC_DEBUG("Mod subscribed to event type: {}", eventType);
        return modSubscriptionId;
    } catch (const std::exception& e) {
        PC_ERROR("Failed to subscribe to event: {}", e.what());
        return 0;
    }
}

static void API_UnsubscribeEvent(uint32_t subscriptionId) {
    try {
        auto it = g_EventSubscriptions.find(subscriptionId);
        if (it != g_EventSubscriptions.end()) {
            EventBus::getInstance().unsubscribe(it->second);
            g_EventSubscriptions.erase(it);
            PC_DEBUG("Mod unsubscribed from event: {}", subscriptionId);
        }
    } catch (const std::exception& e) {
        PC_ERROR("Failed to unsubscribe from event: {}", e.what());
    }
}

static void API_PublishEvent(uint32_t eventType, const void* eventData) {
    try {
        // TODO: Create appropriate event type based on eventType
        PC_DEBUG("Mod published event type: {}", eventType);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to publish event: {}", e.what());
    }
}

// World API implementations
static bool API_GetChunkLoaded(int32_t chunkX, int32_t chunkZ) {
    try {
        // TODO: Implement via ChunkManager when available
        PC_DEBUG("Mod queried chunk loaded: ({}, {})", chunkX, chunkZ);
        return false;
    } catch (...) {
        return false;
    }
}

static int64_t API_GetWorldSeed() {
    try {
        // TODO: Implement via World when available
        PC_DEBUG("Mod queried world seed");
        return 0;
    } catch (...) {
        return 0;
    }
}

// Logging API implementations
static void API_LogInfo(const char* message) {
    if (message) {
        PC_INFO("[MOD] {}", message);
    }
}

static void API_LogWarn(const char* message) {
    if (message) {
        PC_WARN("[MOD] {}", message);
    }
}

static void API_LogError(const char* message) {
    if (message) {
        PC_ERROR("[MOD] {}", message);
    }
}

// Config API implementations
static int32_t API_GetConfigInt(const char* key, int32_t defaultValue) {
    try {
        return Config::getInstance().get_int(key, defaultValue);
    } catch (...) {
        return defaultValue;
    }
}

static float API_GetConfigFloat(const char* key, float defaultValue) {
    try {
        return Config::getInstance().get_float(key, defaultValue);
    } catch (...) {
        return defaultValue;
    }
}

static bool API_GetConfigString(const char* key, const char* defaultValue, char* outBuffer, size_t bufferSize) {
    try {
        std::string value = Config::getInstance().get_string(key, defaultValue ? defaultValue : "");
        if (outBuffer && bufferSize > 0) {
            std::strncpy(outBuffer, value.c_str(), bufferSize - 1);
            outBuffer[bufferSize - 1] = '\0';
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

static void API_SetConfigInt(const char* key, int32_t value) {
    try {
        Config::getInstance().set_int(key, value);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to set config int: {}", e.what());
    }
}

static void API_SetConfigFloat(const char* key, float value) {
    try {
        Config::getInstance().set_float(key, value);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to set config float: {}", e.what());
    }
}

// Create and populate ModAPI struct
ModAPI createModAPI() {
    ModAPI api;
    
    // Block API
    api.registerBlock = API_RegisterBlock;
    api.getBlockID = API_GetBlockID;
    api.getBlockName = API_GetBlockName;
    api.setBlockAt = API_SetBlockAt;
    api.getBlockAt = API_GetBlockAt;
    
    // Entity API
    api.spawnEntity = API_SpawnEntity;
    api.destroyEntity = API_DestroyEntity;
    api.getEntityPosition = API_GetEntityPosition;
    api.setEntityPosition = API_SetEntityPosition;
    
    // Event API
    api.subscribeEvent = API_SubscribeEvent;
    api.unsubscribeEvent = API_UnsubscribeEvent;
    api.publishEvent = API_PublishEvent;
    
    // World API
    api.getChunkLoaded = API_GetChunkLoaded;
    api.getWorldSeed = API_GetWorldSeed;
    
    // Logging API
    api.logInfo = API_LogInfo;
    api.logWarn = API_LogWarn;
    api.logError = API_LogError;
    
    // Config API
    api.getConfigInt = API_GetConfigInt;
    api.getConfigFloat = API_GetConfigFloat;
    api.getConfigString = API_GetConfigString;
    api.setConfigInt = API_SetConfigInt;
    api.setConfigFloat = API_SetConfigFloat;
    
    return api;
}

} // namespace PoorCraft

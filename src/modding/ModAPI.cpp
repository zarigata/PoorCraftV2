#include "poorcraft/modding/ModAPI.h"
#include "poorcraft/modding/ModEvents.h"
#include "poorcraft/network/NetworkEvents.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/BlockType.h"
#include "poorcraft/world/World.h"
#include "poorcraft/world/ChunkManager.h"
#include "poorcraft/world/ChunkCoord.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/entity/components/NetworkIdentity.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/core/Event.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"

#include <cstring>
#include <unordered_map>

namespace PoorCraft {

// Storage for event subscriptions (for cleanup)
static std::unordered_map<uint32_t, size_t> g_EventSubscriptions;
static uint32_t g_NextSubscriptionId = 1;

// Current mod context (set by ModManager during initialization)
static std::vector<uint32_t>* g_CurrentModSubscriptions = nullptr;

// Engine system pointers (injected by ModManager)
static EntityManager* g_EntityManager = nullptr;
static World* g_World = nullptr;
static ChunkManager* g_ChunkManager = nullptr;

void ModAPI_SetCurrentModContext(std::vector<uint32_t>* subscriptions) {
    g_CurrentModSubscriptions = subscriptions;
}

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
        if (!g_World) {
            PC_WARN("Mod attempted to set block but World is not available");
            return false;
        }
        return g_World->setBlockAt(worldX, worldY, worldZ, blockId, 0);
    } catch (...) {
        return false;
    }
}

static uint16_t API_GetBlockAt(int32_t worldX, int32_t worldY, int32_t worldZ) {
    try {
        if (!g_World) {
            PC_WARN("Mod attempted to get block but World is not available");
            return 0;
        }
        return g_World->getBlockAt(worldX, worldY, worldZ);
    } catch (...) {
        return 0;
    }
}

// Entity API implementations
static uint32_t API_SpawnEntity(const char* name, float x, float y, float z) {
    try {
        if (!g_EntityManager) {
            PC_ERROR("Mod attempted to spawn entity but EntityManager is not available");
            return 0;
        }
        Entity& entity = g_EntityManager->createEntity(name);
        
        // Add Transform component with position
        if (!entity.hasComponent<Transform>()) {
            auto& transform = entity.addComponent<Transform>();
            transform.position = glm::vec3(x, y, z);
        }
        
        // Add NetworkIdentity for client sync
        if (!entity.hasComponent<NetworkIdentity>()) {
            auto& netId = entity.addComponent<NetworkIdentity>();
            netId.setNetworkId(entity.getId());
            netId.setServerControlled(true);
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
        if (!g_EntityManager) {
            PC_ERROR("Mod attempted to destroy entity but EntityManager is not available");
            return false;
        }
        g_EntityManager->destroyEntity(static_cast<EntityID>(entityId));
        PC_DEBUG("Mod destroyed entity: {}", entityId);
        return true;
    } catch (...) {
        return false;
    }
}

static bool API_GetEntityPosition(uint32_t entityId, float* outX, float* outY, float* outZ) {
    try {
        if (!g_EntityManager) {
            return false;
        }
        Entity* entity = g_EntityManager->getEntity(static_cast<EntityID>(entityId));
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
        if (!g_EntityManager) {
            return false;
        }
        Entity* entity = g_EntityManager->getEntity(static_cast<EntityID>(entityId));
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
        // Create wrapper lambda that calls C callback with proper payload
        auto listener = [callback, userData, eventType](Event& event) {
            if (!callback) return;
            
            // Populate C struct based on event type
            switch (static_cast<EventType>(eventType)) {
                case EventType::PlayerJoined: {
                    auto& e = static_cast<PlayerJoinedEvent&>(event);
                    PlayerJoinedEventData data;
                    data.playerId = static_cast<uint32_t>(e.getPlayerId());
                    std::strncpy(data.playerName, e.getPlayerName().c_str(), 63);
                    data.playerName[63] = '\0';
                    data.positionX = e.getPosition().x;
                    data.positionY = e.getPosition().y;
                    data.positionZ = e.getPosition().z;
                    callback(&data, userData);
                    break;
                }
                case EventType::PlayerLeft: {
                    auto& e = static_cast<PlayerLeftEvent&>(event);
                    PlayerLeftEventData data;
                    data.playerId = static_cast<uint32_t>(e.getPlayerId());
                    std::strncpy(data.playerName, e.getPlayerName().c_str(), 63);
                    data.playerName[63] = '\0';
                    std::strncpy(data.reason, e.getReason().c_str(), 127);
                    data.reason[127] = '\0';
                    callback(&data, userData);
                    break;
                }
                case EventType::BlockPlaced: {
                    auto& e = static_cast<BlockPlacedEvent&>(event);
                    BlockPlacedEventData data;
                    data.x = e.getX();
                    data.y = e.getY();
                    data.z = e.getZ();
                    data.blockId = e.getBlockId();
                    data.playerId = static_cast<uint32_t>(e.getPlayerId());
                    data.previousBlockId = e.getPreviousBlockId();
                    callback(&data, userData);
                    break;
                }
                case EventType::BlockBroken: {
                    auto& e = static_cast<BlockBrokenEvent&>(event);
                    BlockBrokenEventData data;
                    data.x = e.getX();
                    data.y = e.getY();
                    data.z = e.getZ();
                    data.blockId = e.getBlockId();
                    data.playerId = static_cast<uint32_t>(e.getPlayerId());
                    callback(&data, userData);
                    break;
                }
                case EventType::EntitySpawned: {
                    auto& e = static_cast<EntitySpawnedEvent&>(event);
                    EntitySpawnedEventData data;
                    data.entityId = static_cast<uint32_t>(e.getEntityId());
                    std::strncpy(data.entityName, e.getEntityName().c_str(), 63);
                    data.entityName[63] = '\0';
                    data.positionX = e.getPosition().x;
                    data.positionY = e.getPosition().y;
                    data.positionZ = e.getPosition().z;
                    data.spawnedBy = static_cast<uint32_t>(e.getSpawnedBy());
                    callback(&data, userData);
                    break;
                }
                case EventType::EntityDestroyed: {
                    auto& e = static_cast<EntityDestroyedEvent&>(event);
                    EntityDestroyedEventData data;
                    data.entityId = static_cast<uint32_t>(e.getEntityId());
                    std::strncpy(data.reason, e.getReason().c_str(), 127);
                    data.reason[127] = '\0';
                    callback(&data, userData);
                    break;
                }
                case EventType::PlayerInteract: {
                    auto& e = static_cast<PlayerInteractEvent&>(event);
                    PlayerInteractEventData data;
                    data.playerId = static_cast<uint32_t>(e.getPlayerId());
                    data.targetX = e.getTargetX();
                    data.targetY = e.getTargetY();
                    data.targetZ = e.getTargetZ();
                    data.targetBlockId = e.getTargetBlockId();
                    data.interactionType = static_cast<uint8_t>(e.getInteractionType());
                    callback(&data, userData);
                    break;
                }
                case EventType::ChunkGenerated: {
                    auto& e = static_cast<ChunkGeneratedEvent&>(event);
                    ChunkGeneratedEventData data;
                    data.chunkX = e.getChunkX();
                    data.chunkZ = e.getChunkZ();
                    callback(&data, userData);
                    break;
                }
                default:
                    // For unsupported event types, pass nullptr
                    callback(nullptr, userData);
                    break;
            }
        };
        
        size_t busSubscriptionId = EventBus::getInstance().subscribe(static_cast<EventType>(eventType), listener);
        
        uint32_t modSubscriptionId = g_NextSubscriptionId++;
        g_EventSubscriptions[modSubscriptionId] = busSubscriptionId;
        
        // Track subscription in current mod context
        if (g_CurrentModSubscriptions) {
            g_CurrentModSubscriptions->push_back(modSubscriptionId);
        }
        
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
        } else {
            // Gracefully ignore unknown IDs (may have been already unsubscribed)
            PC_DEBUG("Attempted to unsubscribe unknown subscription ID: {}", subscriptionId);
        }
    } catch (const std::exception& e) {
        PC_ERROR("Failed to unsubscribe from event: {}", e.what());
    }
}

static void API_PublishEvent(uint32_t eventType, const void* eventData) {
    try {
        if (!eventData) {
            PC_ERROR("Cannot publish event with null data");
            return;
        }
        
        // Create appropriate event type based on eventType and publish
        switch (static_cast<EventType>(eventType)) {
            case EventType::BlockPlaced: {
                auto* data = static_cast<const BlockPlacedEventData*>(eventData);
                BlockPlacedEvent event(data->x, data->y, data->z, data->blockId, 
                                      static_cast<EntityID>(data->playerId), data->previousBlockId);
                EventBus::getInstance().publish(event);
                break;
            }
            case EventType::BlockBroken: {
                auto* data = static_cast<const BlockBrokenEventData*>(eventData);
                BlockBrokenEvent event(data->x, data->y, data->z, data->blockId, 
                                      static_cast<EntityID>(data->playerId));
                EventBus::getInstance().publish(event);
                break;
            }
            case EventType::EntitySpawned: {
                auto* data = static_cast<const EntitySpawnedEventData*>(eventData);
                glm::vec3 pos(data->positionX, data->positionY, data->positionZ);
                EntitySpawnedEvent event(static_cast<EntityID>(data->entityId), 
                                        std::string(data->entityName), pos, 
                                        static_cast<EntityID>(data->spawnedBy));
                EventBus::getInstance().publish(event);
                break;
            }
            case EventType::EntityDestroyed: {
                auto* data = static_cast<const EntityDestroyedEventData*>(eventData);
                EntityDestroyedEvent event(static_cast<EntityID>(data->entityId), 
                                          std::string(data->reason));
                EventBus::getInstance().publish(event);
                break;
            }
            case EventType::PlayerInteract: {
                auto* data = static_cast<const PlayerInteractEventData*>(eventData);
                PlayerInteractEvent event(static_cast<EntityID>(data->playerId),
                                         data->targetX, data->targetY, data->targetZ,
                                         data->targetBlockId, 
                                         static_cast<InteractionType>(data->interactionType));
                EventBus::getInstance().publish(event);
                break;
            }
            case EventType::ChunkGenerated: {
                auto* data = static_cast<const ChunkGeneratedEventData*>(eventData);
                ChunkGeneratedEvent event(data->chunkX, data->chunkZ);
                EventBus::getInstance().publish(event);
                break;
            }
            default:
                PC_WARN("Mod attempted to publish unsupported event type: {}", eventType);
                break;
        }
        
        PC_DEBUG("Mod published event type: {}", eventType);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to publish event: {}", e.what());
    }
}

// World API implementations
static bool API_GetChunkLoaded(int32_t chunkX, int32_t chunkZ) {
    try {
        if (!g_ChunkManager) {
            PC_WARN("Mod queried chunk loaded but ChunkManager is not available");
            return false;
        }
        ChunkCoord coord(chunkX, chunkZ);
        return g_ChunkManager->hasChunk(coord);
    } catch (...) {
        return false;
    }
}

static int64_t API_GetWorldSeed() {
    try {
        // TODO: Implement via World when available
        PC_WARN("Mod queried world seed - not yet implemented, returning 0");
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
        return poorcraft::Config::get_instance().get_int(key, defaultValue);
    } catch (...) {
        return defaultValue;
    }
}

static float API_GetConfigFloat(const char* key, float defaultValue) {
    try {
        return poorcraft::Config::get_instance().get_float(key, defaultValue);
    } catch (...) {
        return defaultValue;
    }
}

static bool API_GetConfigString(const char* key, const char* defaultValue, char* outBuffer, size_t bufferSize) {
    try {
        std::string value = poorcraft::Config::get_instance().get_string(key, defaultValue ? defaultValue : "");
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
        poorcraft::Config::get_instance().set_int(key, value);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to set config int: {}", e.what());
    }
}

static void API_SetConfigFloat(const char* key, float value) {
    try {
        poorcraft::Config::get_instance().set_float(key, value);
    } catch (const std::exception& e) {
        PC_ERROR("Failed to set config float: {}", e.what());
    }
}

// Create and populate ModAPI struct
ModAPI createModAPI(EntityManager* entityManager, World* world, ChunkManager* chunkManager) {
    // Store injected pointers
    g_EntityManager = entityManager;
    g_World = world;
    g_ChunkManager = chunkManager;
    
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

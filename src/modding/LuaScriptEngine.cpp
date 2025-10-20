#include "poorcraft/modding/LuaScriptEngine.h"
#include "poorcraft/modding/ModAPI.h"
#include "poorcraft/modding/ModInfo.h"
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

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <glm/glm.hpp>

namespace PoorCraft {

LuaScriptEngine::LuaScriptEngine()
    : m_State(nullptr)
    , m_EntityManager(nullptr)
    , m_World(nullptr)
    , m_ChunkManager(nullptr)
{
}

LuaScriptEngine::~LuaScriptEngine() {
    shutdown();
}

void LuaScriptEngine::setEntityManager(EntityManager* entityManager) {
    m_EntityManager = entityManager;
}

void LuaScriptEngine::setWorld(World* world) {
    m_World = world;
}

void LuaScriptEngine::setChunkManager(ChunkManager* chunkManager) {
    m_ChunkManager = chunkManager;
}

void LuaScriptEngine::initialize() {
    PC_INFO("Initializing Lua script engine...");

    // Create Lua state
    m_State = std::make_unique<sol::state>();

    // Open standard libraries (exclude io/os/package for security)
    m_State->open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::coroutine
    );

    // Set global engine version
    (*m_State)["ENGINE_VERSION"] = "0.1.0";
    (*m_State)["API_VERSION"] = ENGINE_API_VERSION;

    // Register engine bindings
    registerEngineBindings();

    PC_INFO("Lua script engine initialized");
}

void LuaScriptEngine::shutdown() {
    if (m_State) {
        PC_INFO("Shutting down Lua script engine...");
        m_State.reset();
        m_ScriptPaths.clear();
    }
}

bool LuaScriptEngine::executeScript(const std::string& scriptPath) {
    if (!m_State) {
        PC_ERROR("Lua state not initialized");
        return false;
    }

    try {
        sol::protected_function_result result = m_State->script_file(scriptPath);
        
        if (!result.valid()) {
            sol::error err = result;
            handleLuaError(err.what());
            return false;
        }

        m_ScriptPaths.push_back(scriptPath);
        PC_INFO("Executed Lua script: {}", scriptPath);
        return true;

    } catch (const std::exception& e) {
        handleLuaError(e.what());
        return false;
    }
}

bool LuaScriptEngine::executeString(const std::string& luaCode) {
    if (!m_State) {
        PC_ERROR("Lua state not initialized");
        return false;
    }

    try {
        sol::protected_function_result result = m_State->script(luaCode);
        
        if (!result.valid()) {
            sol::error err = result;
            handleLuaError(err.what());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        handleLuaError(e.what());
        return false;
    }
}

template<typename... Args>
bool LuaScriptEngine::callFunction(const std::string& functionName, Args&&... args) {
    if (!m_State) {
        PC_ERROR("Lua state not initialized");
        return false;
    }

    try {
        sol::protected_function func = (*m_State)[functionName];
        
        if (!func.valid()) {
            PC_DEBUG("Lua function '{}' not found", functionName);
            return false;
        }

        sol::protected_function_result result = func(std::forward<Args>(args)...);
        
        if (!result.valid()) {
            sol::error err = result;
            handleLuaError(err.what());
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        handleLuaError(e.what());
        return false;
    }
}

// Explicit template instantiation for common types
template bool LuaScriptEngine::callFunction<float>(const std::string&, float&&);
template bool LuaScriptEngine::callFunction<>(const std::string&);

void LuaScriptEngine::registerEngineBindings() {
    PC_DEBUG("Registering Lua engine bindings...");

    // Register glm::vec3 type
    m_State->new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    // Create Block namespace
    sol::table blockNamespace = m_State->create_named_table("Block");
    
    blockNamespace["registerBlock"] = [](sol::table blockDef) -> uint16_t {
        try {
            std::string name = blockDef.get_or<std::string>("name", "");
            bool isSolid = blockDef.get_or<bool>("isSolid", true);
            bool isOpaque = blockDef.get_or<bool>("isOpaque", true);
            bool isTransparent = blockDef.get_or<bool>("isTransparent", false);
            std::string textureName = blockDef.get_or<std::string>("textureName", "stone");
            float hardness = blockDef.get_or<float>("hardness", 1.0f);

            BlockType block;
            block.setName(name)
                 .setSolid(isSolid)
                 .setOpaque(isOpaque)
                 .setTransparent(isTransparent)
                 .setTextureAllFaces(textureName)
                 .setHardness(hardness);

            uint16_t id = BlockRegistry::getInstance().registerBlock(block);
            PC_INFO("Lua mod registered block: {} (ID: {})", name, id);
            return id;
        } catch (const std::exception& e) {
            PC_ERROR("Lua Block.registerBlock failed: {}", e.what());
            return 0;
        }
    };

    blockNamespace["getBlockID"] = [](const std::string& name) -> uint16_t {
        try {
            return BlockRegistry::getInstance().getBlockID(name);
        } catch (...) {
            return 0;
        }
    };

    blockNamespace["getBlockName"] = [](uint16_t id) -> std::string {
        try {
            const BlockType& block = BlockRegistry::getInstance().getBlock(id);
            return block.name;
        } catch (...) {
            return "";
        }
    };

    blockNamespace["setBlockAt"] = [this](int32_t x, int32_t y, int32_t z, uint16_t blockId) -> bool {
        if (!m_World) {
            PC_WARN("Lua mod attempted to set block but World is not available");
            return false;
        }
        return m_World->setBlockAt(x, y, z, blockId, 0);
    };

    blockNamespace["getBlockAt"] = [this](int32_t x, int32_t y, int32_t z) -> uint16_t {
        if (!m_World) {
            PC_WARN("Lua mod attempted to get block but World is not available");
            return 0;
        }
        return m_World->getBlockAt(x, y, z);
    };

    // Create Entity namespace
    sol::table entityNamespace = m_State->create_named_table("Entity");

    entityNamespace["spawn"] = [this](const std::string& name, float x, float y, float z) -> uint32_t {
        try {
            if (!m_EntityManager) {
                PC_ERROR("Lua mod attempted to spawn entity but EntityManager is not available");
                return 0;
            }
            Entity& entity = m_EntityManager->createEntity(name);
            
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

            PC_INFO("Lua mod spawned entity: {} (ID: {}) at ({}, {}, {})", 
                    name, entity.getId(), x, y, z);
            return static_cast<uint32_t>(entity.getId());
        } catch (const std::exception& e) {
            PC_ERROR("Lua Entity.spawn failed: {}", e.what());
            return 0;
        }
    };

    entityNamespace["destroy"] = [this](uint32_t entityId) -> bool {
        try {
            if (!m_EntityManager) {
                PC_ERROR("Lua mod attempted to destroy entity but EntityManager is not available");
                return false;
            }
            m_EntityManager->destroyEntity(static_cast<EntityID>(entityId));
            return true;
        } catch (...) {
            return false;
        }
    };

    entityNamespace["getPosition"] = [this](uint32_t entityId) -> sol::optional<glm::vec3> {
        try {
            if (!m_EntityManager) {
                return sol::nullopt;
            }
            Entity* entity = m_EntityManager->getEntity(static_cast<EntityID>(entityId));
            if (!entity) return sol::nullopt;

            Transform* transform = entity->getComponent<Transform>();
            if (!transform) return sol::nullopt;

            return transform->position;
        } catch (...) {
            return sol::nullopt;
        }
    };

    entityNamespace["setPosition"] = [this](uint32_t entityId, float x, float y, float z) -> bool {
        try {
            if (!m_EntityManager) {
                return false;
            }
            Entity* entity = m_EntityManager->getEntity(static_cast<EntityID>(entityId));
            if (!entity) return false;

            Transform* transform = entity->getComponent<Transform>();
            if (!transform) return false;

            transform->position = glm::vec3(x, y, z);
            return true;
        } catch (...) {
            return false;
        }
    };

    // Create EventBus namespace
    sol::table eventBusNamespace = m_State->create_named_table("EventBus");

    eventBusNamespace["subscribe"] = [this](const std::string& eventTypeName, sol::function callback) -> uint32_t {
        try {
            // Map event type name to EventType enum
            EventType eventType = EventType::None;
            if (eventTypeName == "PlayerJoined") eventType = EventType::PlayerJoined;
            else if (eventTypeName == "PlayerLeft") eventType = EventType::PlayerLeft;
            else if (eventTypeName == "BlockPlaced") eventType = EventType::BlockPlaced;
            else if (eventTypeName == "BlockBroken") eventType = EventType::BlockBroken;
            else if (eventTypeName == "EntitySpawned") eventType = EventType::EntitySpawned;
            else if (eventTypeName == "EntityDestroyed") eventType = EventType::EntityDestroyed;
            else if (eventTypeName == "PlayerInteract") eventType = EventType::PlayerInteract;
            else if (eventTypeName == "ChunkGenerated") eventType = EventType::ChunkGenerated;
            else {
                PC_WARN("Lua mod subscribed to unknown event type: {}", eventTypeName);
                return 0;
            }
            
            auto listener = [this, callback, eventType](Event& event) mutable {
                try {
                    // Create Lua table with event data based on type
                    sol::table eventData = m_State->create_table();
                    
                    switch (eventType) {
                        case EventType::PlayerJoined: {
                            auto& e = static_cast<PlayerJoinedEvent&>(event);
                            eventData["playerId"] = static_cast<uint32_t>(e.getPlayerId());
                            eventData["playerName"] = e.getPlayerName();
                            eventData["position"] = e.getPosition();
                            break;
                        }
                        case EventType::PlayerLeft: {
                            auto& e = static_cast<PlayerLeftEvent&>(event);
                            eventData["playerId"] = static_cast<uint32_t>(e.getPlayerId());
                            eventData["playerName"] = e.getPlayerName();
                            eventData["reason"] = e.getReason();
                            break;
                        }
                        case EventType::BlockPlaced: {
                            auto& e = static_cast<BlockPlacedEvent&>(event);
                            eventData["x"] = e.getX();
                            eventData["y"] = e.getY();
                            eventData["z"] = e.getZ();
                            eventData["blockId"] = e.getBlockId();
                            eventData["playerId"] = static_cast<uint32_t>(e.getPlayerId());
                            eventData["previousBlockId"] = e.getPreviousBlockId();
                            break;
                        }
                        case EventType::BlockBroken: {
                            auto& e = static_cast<BlockBrokenEvent&>(event);
                            eventData["x"] = e.getX();
                            eventData["y"] = e.getY();
                            eventData["z"] = e.getZ();
                            eventData["blockId"] = e.getBlockId();
                            eventData["playerId"] = static_cast<uint32_t>(e.getPlayerId());
                            break;
                        }
                        case EventType::EntitySpawned: {
                            auto& e = static_cast<EntitySpawnedEvent&>(event);
                            eventData["entityId"] = static_cast<uint32_t>(e.getEntityId());
                            eventData["entityName"] = e.getEntityName();
                            eventData["position"] = e.getPosition();
                            eventData["spawnedBy"] = static_cast<uint32_t>(e.getSpawnedBy());
                            break;
                        }
                        case EventType::EntityDestroyed: {
                            auto& e = static_cast<EntityDestroyedEvent&>(event);
                            eventData["entityId"] = static_cast<uint32_t>(e.getEntityId());
                            eventData["reason"] = e.getReason();
                            break;
                        }
                        case EventType::PlayerInteract: {
                            auto& e = static_cast<PlayerInteractEvent&>(event);
                            eventData["playerId"] = static_cast<uint32_t>(e.getPlayerId());
                            eventData["targetX"] = e.getTargetX();
                            eventData["targetY"] = e.getTargetY();
                            eventData["targetZ"] = e.getTargetZ();
                            eventData["targetBlockId"] = e.getTargetBlockId();
                            eventData["interactionType"] = static_cast<uint8_t>(e.getInteractionType());
                            break;
                        }
                        case EventType::ChunkGenerated: {
                            auto& e = static_cast<ChunkGeneratedEvent&>(event);
                            eventData["chunkX"] = e.getChunkX();
                            eventData["chunkZ"] = e.getChunkZ();
                            break;
                        }
                        default:
                            break;
                    }
                    
                    callback(eventData);
                } catch (const std::exception& e) {
                    PC_ERROR("Lua event callback failed: {}", e.what());
                }
            };

            size_t subId = EventBus::getInstance().subscribe(eventType, listener);
            PC_DEBUG("Lua mod subscribed to event: {}", eventTypeName);
            return static_cast<uint32_t>(subId);
        } catch (const std::exception& e) {
            PC_ERROR("Lua EventBus.subscribe failed: {}", e.what());
            return 0;
        }
    };

    eventBusNamespace["unsubscribe"] = [](uint32_t subscriptionId) {
        try {
            EventBus::getInstance().unsubscribe(static_cast<size_t>(subscriptionId));
        } catch (const std::exception& e) {
            PC_ERROR("Lua EventBus.unsubscribe failed: {}", e.what());
        }
    };
    
    eventBusNamespace["publish"] = [this](const std::string& eventTypeName, sol::table eventData) {
        try {
            // Map event type name and publish
            if (eventTypeName == "BlockPlaced") {
                BlockPlacedEvent event(
                    eventData.get_or("x", 0),
                    eventData.get_or("y", 0),
                    eventData.get_or("z", 0),
                    eventData.get_or<uint16_t>("blockId", 0),
                    static_cast<EntityID>(eventData.get_or<uint32_t>("playerId", 0)),
                    eventData.get_or<uint16_t>("previousBlockId", 0)
                );
                EventBus::getInstance().publish(event);
            } else if (eventTypeName == "BlockBroken") {
                BlockBrokenEvent event(
                    eventData.get_or("x", 0),
                    eventData.get_or("y", 0),
                    eventData.get_or("z", 0),
                    eventData.get_or<uint16_t>("blockId", 0),
                    static_cast<EntityID>(eventData.get_or<uint32_t>("playerId", 0))
                );
                EventBus::getInstance().publish(event);
            } else {
                PC_WARN("Lua mod attempted to publish unsupported event type: {}", eventTypeName);
            }
        } catch (const std::exception& e) {
            PC_ERROR("Lua EventBus.publish failed: {}", e.what());
        }
    };

    // Create World namespace
    sol::table worldNamespace = m_State->create_named_table("World");

    worldNamespace["getBlockAt"] = blockNamespace["getBlockAt"];
    worldNamespace["setBlockAt"] = blockNamespace["setBlockAt"];
    
    worldNamespace["getSeed"] = []() -> int64_t {
        // TODO: Implement via World
        PC_WARN("Lua mod queried world seed - not yet implemented, returning 0");
        return 0;
    };

    worldNamespace["getChunkLoaded"] = [this](int32_t chunkX, int32_t chunkZ) -> bool {
        if (!m_ChunkManager) {
            PC_WARN("Lua mod queried chunk loaded but ChunkManager is not available");
            return false;
        }
        ChunkCoord coord(chunkX, chunkZ);
        return m_ChunkManager->hasChunk(coord);
    };

    // Create Config namespace
    sol::table configNamespace = m_State->create_named_table("Config");

    configNamespace["getInt"] = [](const std::string& key, int32_t defaultValue) -> int32_t {
        try {
            return poorcraft::Config::get_instance().get_int(key, defaultValue);
        } catch (...) {
            return defaultValue;
        }
    };

    configNamespace["getFloat"] = [](const std::string& key, float defaultValue) -> float {
        try {
            return poorcraft::Config::get_instance().get_float(key, defaultValue);
        } catch (...) {
            return defaultValue;
        }
    };

    configNamespace["getString"] = [](const std::string& key, const std::string& defaultValue) -> std::string {
        try {
            return poorcraft::Config::get_instance().get_string(key, defaultValue);
        } catch (...) {
            return defaultValue;
        }
    };

    configNamespace["setInt"] = [](const std::string& key, int32_t value) {
        try {
            poorcraft::Config::get_instance().set_int(key, value);
        } catch (const std::exception& e) {
            PC_ERROR("Lua Config.setInt failed: {}", e.what());
        }
    };

    PC_DEBUG("Lua engine bindings registered");
}

void LuaScriptEngine::handleLuaError(const std::string& error) {
    PC_ERROR("Lua error: {}", error);
    // TODO: Extract and format stack trace
}

} // namespace PoorCraft

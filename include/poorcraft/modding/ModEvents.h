#pragma once

#include "poorcraft/core/Event.h"
#include "poorcraft/entity/Entity.h"

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

namespace PoorCraft {

// Mod lifecycle events
class ModLoadedEvent : public Event {
public:
    ModLoadedEvent(const std::string& modId, const std::string& modName, const std::string& modVersion)
        : m_ModId(modId), m_ModName(modName), m_ModVersion(modVersion) {}

    const std::string& getModId() const { return m_ModId; }
    const std::string& getModName() const { return m_ModName; }
    const std::string& getModVersion() const { return m_ModVersion; }

    std::string toString() const override {
        return "ModLoadedEvent: " + m_ModName + " v" + m_ModVersion + " (" + m_ModId + ")";
    }

    EVENT_CLASS_TYPE(ModLoaded)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    std::string m_ModId;
    std::string m_ModName;
    std::string m_ModVersion;
};

class ModUnloadedEvent : public Event {
public:
    ModUnloadedEvent(const std::string& modId, const std::string& reason)
        : m_ModId(modId), m_Reason(reason) {}

    const std::string& getModId() const { return m_ModId; }
    const std::string& getReason() const { return m_Reason; }

    std::string toString() const override {
        return "ModUnloadedEvent: " + m_ModId + " (" + m_Reason + ")";
    }

    EVENT_CLASS_TYPE(ModUnloaded)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    std::string m_ModId;
    std::string m_Reason;
};

class ModReloadedEvent : public Event {
public:
    explicit ModReloadedEvent(const std::string& modId)
        : m_ModId(modId) {}

    const std::string& getModId() const { return m_ModId; }

    std::string toString() const override {
        return "ModReloadedEvent: " + m_ModId;
    }

    EVENT_CLASS_TYPE(ModReloaded)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    std::string m_ModId;
};

// Gameplay events for mods
class BlockPlacedEvent : public Event {
public:
    BlockPlacedEvent(int32_t x, int32_t y, int32_t z, uint16_t blockId, EntityID playerId, uint16_t previousBlockId)
        : m_X(x), m_Y(y), m_Z(z), m_BlockId(blockId), m_PlayerId(playerId), m_PreviousBlockId(previousBlockId) {}

    int32_t getX() const { return m_X; }
    int32_t getY() const { return m_Y; }
    int32_t getZ() const { return m_Z; }
    uint16_t getBlockId() const { return m_BlockId; }
    EntityID getPlayerId() const { return m_PlayerId; }
    uint16_t getPreviousBlockId() const { return m_PreviousBlockId; }

    std::string toString() const override {
        return "BlockPlacedEvent: Block " + std::to_string(m_BlockId) + 
               " placed at (" + std::to_string(m_X) + ", " + std::to_string(m_Y) + ", " + std::to_string(m_Z) + 
               ") by player " + std::to_string(m_PlayerId);
    }

    EVENT_CLASS_TYPE(BlockPlaced)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    int32_t m_X, m_Y, m_Z;
    uint16_t m_BlockId;
    EntityID m_PlayerId;
    uint16_t m_PreviousBlockId;
};

class BlockBrokenEvent : public Event {
public:
    BlockBrokenEvent(int32_t x, int32_t y, int32_t z, uint16_t blockId, EntityID playerId)
        : m_X(x), m_Y(y), m_Z(z), m_BlockId(blockId), m_PlayerId(playerId) {}

    int32_t getX() const { return m_X; }
    int32_t getY() const { return m_Y; }
    int32_t getZ() const { return m_Z; }
    uint16_t getBlockId() const { return m_BlockId; }
    EntityID getPlayerId() const { return m_PlayerId; }

    std::string toString() const override {
        return "BlockBrokenEvent: Block " + std::to_string(m_BlockId) + 
               " broken at (" + std::to_string(m_X) + ", " + std::to_string(m_Y) + ", " + std::to_string(m_Z) + 
               ") by player " + std::to_string(m_PlayerId);
    }

    EVENT_CLASS_TYPE(BlockBroken)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    int32_t m_X, m_Y, m_Z;
    uint16_t m_BlockId;
    EntityID m_PlayerId;
};

class EntitySpawnedEvent : public Event {
public:
    EntitySpawnedEvent(EntityID entityId, const std::string& entityName, const glm::vec3& position, EntityID spawnedBy)
        : m_EntityId(entityId), m_EntityName(entityName), m_Position(position), m_SpawnedBy(spawnedBy) {}

    EntityID getEntityId() const { return m_EntityId; }
    const std::string& getEntityName() const { return m_EntityName; }
    const glm::vec3& getPosition() const { return m_Position; }
    EntityID getSpawnedBy() const { return m_SpawnedBy; }

    std::string toString() const override {
        return "EntitySpawnedEvent: " + m_EntityName + " (ID: " + std::to_string(m_EntityId) + 
               ") at (" + std::to_string(m_Position.x) + ", " + std::to_string(m_Position.y) + ", " + std::to_string(m_Position.z) + ")";
    }

    EVENT_CLASS_TYPE(EntitySpawned)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    EntityID m_EntityId;
    std::string m_EntityName;
    glm::vec3 m_Position;
    EntityID m_SpawnedBy;
};

class EntityDestroyedEvent : public Event {
public:
    EntityDestroyedEvent(EntityID entityId, const std::string& reason)
        : m_EntityId(entityId), m_Reason(reason) {}

    EntityID getEntityId() const { return m_EntityId; }
    const std::string& getReason() const { return m_Reason; }

    std::string toString() const override {
        return "EntityDestroyedEvent: Entity " + std::to_string(m_EntityId) + " (" + m_Reason + ")";
    }

    EVENT_CLASS_TYPE(EntityDestroyed)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    EntityID m_EntityId;
    std::string m_Reason;
};

enum class InteractionType : uint8_t {
    LEFT_CLICK = 0,
    RIGHT_CLICK = 1
};

class PlayerInteractEvent : public Event {
public:
    PlayerInteractEvent(EntityID playerId, int32_t targetX, int32_t targetY, int32_t targetZ, 
                       uint16_t targetBlockId, InteractionType interactionType)
        : m_PlayerId(playerId), m_TargetX(targetX), m_TargetY(targetY), m_TargetZ(targetZ)
        , m_TargetBlockId(targetBlockId), m_InteractionType(interactionType) {}

    EntityID getPlayerId() const { return m_PlayerId; }
    int32_t getTargetX() const { return m_TargetX; }
    int32_t getTargetY() const { return m_TargetY; }
    int32_t getTargetZ() const { return m_TargetZ; }
    uint16_t getTargetBlockId() const { return m_TargetBlockId; }
    InteractionType getInteractionType() const { return m_InteractionType; }

    std::string toString() const override {
        std::string typeStr = (m_InteractionType == InteractionType::LEFT_CLICK) ? "LEFT_CLICK" : "RIGHT_CLICK";
        return "PlayerInteractEvent: Player " + std::to_string(m_PlayerId) + 
               " " + typeStr + " at (" + std::to_string(m_TargetX) + ", " + std::to_string(m_TargetY) + ", " + std::to_string(m_TargetZ) + ")";
    }

    EVENT_CLASS_TYPE(PlayerInteract)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    EntityID m_PlayerId;
    int32_t m_TargetX, m_TargetY, m_TargetZ;
    uint16_t m_TargetBlockId;
    InteractionType m_InteractionType;
};

class ChunkGeneratedEvent : public Event {
public:
    ChunkGeneratedEvent(int32_t chunkX, int32_t chunkZ)
        : m_ChunkX(chunkX), m_ChunkZ(chunkZ) {}

    int32_t getChunkX() const { return m_ChunkX; }
    int32_t getChunkZ() const { return m_ChunkZ; }

    std::string toString() const override {
        return "ChunkGeneratedEvent: Chunk (" + std::to_string(m_ChunkX) + ", " + std::to_string(m_ChunkZ) + ")";
    }

    EVENT_CLASS_TYPE(ChunkGenerated)
    EVENT_CLASS_CATEGORY(EventCategoryMod)

private:
    int32_t m_ChunkX, m_ChunkZ;
};

} // namespace PoorCraft

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "poorcraft/entity/Entity.h"
#include "poorcraft/network/PacketSerializer.h"
#include "poorcraft/network/PacketType.h"

namespace PoorCraft {

struct HandshakeRequestPacket {
    std::uint32_t protocolVersion = 1;
    std::string playerName;
    std::string clientVersion;

    void serialize(PacketWriter& writer) const;
    static HandshakeRequestPacket deserialize(PacketReader& reader);
};

struct HandshakeResponsePacket {
    bool accepted = false;
    EntityID playerId = 0;
    glm::vec3 spawnPosition{0.0f};
    std::int64_t worldSeed = 0;
    std::string serverMessage;

    void serialize(PacketWriter& writer) const;
    static HandshakeResponsePacket deserialize(PacketReader& reader);
};

struct PlayerInputPacket {
    std::uint32_t inputSequence = 0;
    float deltaTime = 0.0f;
    glm::vec3 wishDirection{0.0f};
    bool sprinting = false;
    bool jumpRequested = false;
    bool flyToggle = false;
    bool swimToggle = false;
    float yaw = 0.0f;
    float pitch = 0.0f;
    std::uint8_t actionFlags = 0;

    void serialize(PacketWriter& writer) const;
    static PlayerInputPacket deserialize(PacketReader& reader);
};

struct EntityStateData {
    EntityID entityId = 0;
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    std::uint8_t animationState = 0;
    std::uint8_t stateFlags = 0;
};

struct EntitySnapshotPacket {
    std::uint32_t serverTick = 0;
    std::uint32_t lastConsumedInputSeq = 0;
    std::vector<EntityStateData> entities;

    void serialize(PacketWriter& writer) const;
    static EntitySnapshotPacket deserialize(PacketReader& reader);
};

struct ChunkDataPacket {
    std::int32_t chunkX = 0;
    std::int32_t chunkZ = 0;
    std::uint16_t fragmentId = 0;
    bool isLast = false;
    std::vector<std::uint8_t> blockData;

    void serialize(PacketWriter& writer) const;
    static ChunkDataPacket deserialize(PacketReader& reader);
};

struct ChunkRequestPacket {
    std::int32_t chunkX = 0;
    std::int32_t chunkZ = 0;

    void serialize(PacketWriter& writer) const;
    static ChunkRequestPacket deserialize(PacketReader& reader);
};

struct PlayerJoinPacket {
    EntityID playerId = 0;
    std::string playerName;
    glm::vec3 spawnPosition{0.0f};

    void serialize(PacketWriter& writer) const;
    static PlayerJoinPacket deserialize(PacketReader& reader);
};

struct PlayerLeavePacket {
    EntityID playerId = 0;
    std::string reason;

    void serialize(PacketWriter& writer) const;
    static PlayerLeavePacket deserialize(PacketReader& reader);
};

struct ChatMessagePacket {
    EntityID senderId = 0;
    std::string message;
    std::uint32_t timestamp = 0;

    void serialize(PacketWriter& writer) const;
    static ChatMessagePacket deserialize(PacketReader& reader);
};

struct DisconnectPacket {
    std::string reason;

    void serialize(PacketWriter& writer) const;
    static DisconnectPacket deserialize(PacketReader& reader);
};

struct PingPacket {
    std::uint32_t clientTime = 0;

    void serialize(PacketWriter& writer) const;
    static PingPacket deserialize(PacketReader& reader);
};

struct PongPacket {
    std::uint32_t clientTime = 0;
    std::uint32_t serverTime = 0;

    void serialize(PacketWriter& writer) const;
    static PongPacket deserialize(PacketReader& reader);
};

struct BlockUpdatePacket {
    std::int32_t worldX = 0;
    std::int32_t worldY = 0;
    std::int32_t worldZ = 0;
    std::uint16_t blockId = 0;
    EntityID playerId = 0;

    void serialize(PacketWriter& writer) const;
    static BlockUpdatePacket deserialize(PacketReader& reader);
};

struct PlayerSpawnPacket {
    EntityID playerId = 0;
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};

    void serialize(PacketWriter& writer) const;
    static PlayerSpawnPacket deserialize(PacketReader& reader);
};

} // namespace PoorCraft

#include "poorcraft/network/NetworkPackets.h"

#include <algorithm>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
constexpr float POSITION_PRECISION = 0.01f;
constexpr float ANGLE_PRECISION = 0.01f;
constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
}

void HandshakeRequestPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt32(protocolVersion);
    writer.writeString(playerName);
    writer.writeString(clientVersion);
}

HandshakeRequestPacket HandshakeRequestPacket::deserialize(PacketReader& reader) {
    HandshakeRequestPacket packet;
    packet.protocolVersion = reader.readUInt32();
    packet.playerName = reader.readString();
    packet.clientVersion = reader.readString();
    return packet;
}

void HandshakeResponsePacket::serialize(PacketWriter& writer) const {
    writer.writeUInt8(static_cast<std::uint8_t>(accepted ? 1 : 0));
    writer.writeUInt64(playerId);
    writer.writeVec3(spawnPosition);
    writer.writeUInt64(static_cast<std::uint64_t>(worldSeed));
    writer.writeString(serverMessage);
}

HandshakeResponsePacket HandshakeResponsePacket::deserialize(PacketReader& reader) {
    HandshakeResponsePacket packet;
    packet.accepted = reader.readUInt8() != 0;
    packet.playerId = reader.readUInt64();
    packet.spawnPosition = reader.readVec3();
    packet.worldSeed = static_cast<std::int64_t>(reader.readUInt64());
    packet.serverMessage = reader.readString();
    return packet;
}

void PlayerInputPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt32(inputSequence);
    writer.writeFloat(deltaTime);
    writer.writeVec3Quantized(wishDirection, POSITION_PRECISION);

    std::uint8_t flags = 0;
    if (sprinting) flags |= 1 << 0;
    if (jumpRequested) flags |= 1 << 1;
    if (flyToggle) flags |= 1 << 2;
    if (swimToggle) flags |= 1 << 3;
    writer.writeUInt8(flags);

    const std::int16_t yawQuantized = quantizePositionComponent(yaw * RAD_TO_DEG, ANGLE_PRECISION);
    const std::int16_t pitchQuantized = quantizePositionComponent(pitch * RAD_TO_DEG, ANGLE_PRECISION);
    writer.writeInt16(yawQuantized);
    writer.writeInt16(pitchQuantized);
    writer.writeUInt8(actionFlags);
}

PlayerInputPacket PlayerInputPacket::deserialize(PacketReader& reader) {
    PlayerInputPacket packet;
    packet.inputSequence = reader.readUInt32();
    packet.deltaTime = reader.readFloat();
    packet.wishDirection = reader.readVec3Quantized(POSITION_PRECISION);

    const std::uint8_t flags = reader.readUInt8();
    packet.sprinting = (flags & (1 << 0)) != 0;
    packet.jumpRequested = (flags & (1 << 1)) != 0;
    packet.flyToggle = (flags & (1 << 2)) != 0;
    packet.swimToggle = (flags & (1 << 3)) != 0;

    packet.yaw = dequantizePositionComponent(reader.readInt16(), ANGLE_PRECISION) * DEG_TO_RAD;
    packet.pitch = dequantizePositionComponent(reader.readInt16(), ANGLE_PRECISION) * DEG_TO_RAD;
    packet.actionFlags = reader.readUInt8();
    return packet;
}

void EntitySnapshotPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt32(serverTick);
    writer.writeUInt32(lastConsumedInputSeq);
    writer.writeUInt16(static_cast<std::uint16_t>(entities.size()));
    for (const auto& entity : entities) {
        writer.writeUInt64(entity.entityId);
        writer.writeVec3Quantized(entity.position, POSITION_PRECISION);
        writer.writeVec3Quantized(entity.velocity, POSITION_PRECISION);
        writer.writeQuatCompressed(entity.rotation);
        writer.writeUInt8(entity.animationState);
        writer.writeUInt8(entity.stateFlags);
    }
}

EntitySnapshotPacket EntitySnapshotPacket::deserialize(PacketReader& reader) {
    EntitySnapshotPacket packet;
    packet.serverTick = reader.readUInt32();
    packet.lastConsumedInputSeq = reader.readUInt32();

    const std::uint16_t count = reader.readUInt16();
    packet.entities.reserve(count);
    for (std::uint16_t i = 0; i < count; ++i) {
        EntityStateData state;
        state.entityId = reader.readUInt64();
        state.position = reader.readVec3Quantized(POSITION_PRECISION);
        state.velocity = reader.readVec3Quantized(POSITION_PRECISION);
        state.rotation = reader.readQuatCompressed();
        state.animationState = reader.readUInt8();
        state.stateFlags = reader.readUInt8();
        packet.entities.push_back(state);
    }
    return packet;
}

void ChunkDataPacket::serialize(PacketWriter& writer) const {
    writer.writeInt32(chunkX);
    writer.writeInt32(chunkZ);
    writer.writeUInt32(static_cast<std::uint32_t>(blockData.size()));
    for (std::uint8_t byte : blockData) {
        writer.writeUInt8(byte);
    }
}

ChunkDataPacket ChunkDataPacket::deserialize(PacketReader& reader) {
    ChunkDataPacket packet;
    packet.chunkX = reader.readInt32();
    packet.chunkZ = reader.readInt32();
    const std::uint32_t size = reader.readUInt32();
    packet.blockData.resize(size);
    for (std::uint32_t i = 0; i < size; ++i) {
        packet.blockData[i] = reader.readUInt8();
    }
    return packet;
}

void ChunkRequestPacket::serialize(PacketWriter& writer) const {
    writer.writeInt32(chunkX);
    writer.writeInt32(chunkZ);
}

ChunkRequestPacket ChunkRequestPacket::deserialize(PacketReader& reader) {
    ChunkRequestPacket packet;
    packet.chunkX = reader.readInt32();
    packet.chunkZ = reader.readInt32();
    return packet;
}

void PlayerJoinPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt64(playerId);
    writer.writeString(playerName);
    writer.writeVec3(spawnPosition);
}

PlayerJoinPacket PlayerJoinPacket::deserialize(PacketReader& reader) {
    PlayerJoinPacket packet;
    packet.playerId = reader.readUInt64();
    packet.playerName = reader.readString();
    packet.spawnPosition = reader.readVec3();
    return packet;
}

void PlayerLeavePacket::serialize(PacketWriter& writer) const {
    writer.writeUInt64(playerId);
    writer.writeString(reason);
}

PlayerLeavePacket PlayerLeavePacket::deserialize(PacketReader& reader) {
    PlayerLeavePacket packet;
    packet.playerId = reader.readUInt64();
    packet.reason = reader.readString();
    return packet;
}

void ChatMessagePacket::serialize(PacketWriter& writer) const {
    writer.writeUInt64(senderId);
    writer.writeString(message);
    writer.writeUInt32(timestamp);
}

ChatMessagePacket ChatMessagePacket::deserialize(PacketReader& reader) {
    ChatMessagePacket packet;
    packet.senderId = reader.readUInt64();
    packet.message = reader.readString();
    packet.timestamp = reader.readUInt32();
    return packet;
}

void DisconnectPacket::serialize(PacketWriter& writer) const {
    writer.writeString(reason);
}

DisconnectPacket DisconnectPacket::deserialize(PacketReader& reader) {
    DisconnectPacket packet;
    packet.reason = reader.readString();
    return packet;
}

void PingPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt32(clientTime);
}

PingPacket PingPacket::deserialize(PacketReader& reader) {
    PingPacket packet;
    packet.clientTime = reader.readUInt32();
    return packet;
}

void PongPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt32(clientTime);
    writer.writeUInt32(serverTime);
}

PongPacket PongPacket::deserialize(PacketReader& reader) {
    PongPacket packet;
    packet.clientTime = reader.readUInt32();
    packet.serverTime = reader.readUInt32();
    return packet;
}

void BlockUpdatePacket::serialize(PacketWriter& writer) const {
    writer.writeInt32(worldX);
    writer.writeInt32(worldY);
    writer.writeInt32(worldZ);
    writer.writeUInt16(blockId);
    writer.writeUInt64(playerId);
}

BlockUpdatePacket BlockUpdatePacket::deserialize(PacketReader& reader) {
    BlockUpdatePacket packet;
    packet.worldX = reader.readInt32();
    packet.worldY = reader.readInt32();
    packet.worldZ = reader.readInt32();
    packet.blockId = reader.readUInt16();
    packet.playerId = reader.readUInt64();
    return packet;
}

void PlayerSpawnPacket::serialize(PacketWriter& writer) const {
    writer.writeUInt64(playerId);
    writer.writeVec3(position);
    writer.writeQuat(rotation);
}

PlayerSpawnPacket PlayerSpawnPacket::deserialize(PacketReader& reader) {
    PlayerSpawnPacket packet;
    packet.playerId = reader.readUInt64();
    packet.position = reader.readVec3();
    packet.rotation = reader.readQuat();
    return packet;
}

} // namespace PoorCraft

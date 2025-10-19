#pragma once

#include <cstdint>
#include <string>

namespace PoorCraft {

enum class PacketType : std::uint8_t {
    HANDSHAKE_REQUEST = 0,
    HANDSHAKE_RESPONSE = 1,
    PLAYER_INPUT = 2,
    ENTITY_SNAPSHOT = 3,
    CHUNK_DATA = 4,
    CHUNK_REQUEST = 5,
    PLAYER_JOIN = 6,
    PLAYER_LEAVE = 7,
    CHAT_MESSAGE = 8,
    DISCONNECT = 9,
    PING = 10,
    PONG = 11,
    BLOCK_UPDATE = 12,
    PLAYER_SPAWN = 13
};

struct PacketHeader {
    static constexpr std::size_t SIZE = 1 + 2 + 4 + 4;

    std::uint8_t type;
    std::uint16_t size;
    std::uint32_t sequence;
    std::uint32_t timestamp;

    PacketType getPacketType() const {
        return static_cast<PacketType>(type);
    }

    void setPacketType(PacketType packetType) {
        type = static_cast<std::uint8_t>(packetType);
    }
};

std::string getPacketTypeName(PacketType type);

bool isReliablePacket(PacketType type);

} // namespace PoorCraft

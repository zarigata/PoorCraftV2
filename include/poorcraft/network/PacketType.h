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
    std::uint8_t type;
    std::uint16_t size;
    std::uint32_t sequence;
    std::uint32_t timestamp;
};

std::string getPacketTypeName(PacketType type);

bool isReliablePacket(PacketType type);

// Packet channels:
// Channel 0 (reliable ordered) - handshake, join/leave, chunk data, disconnect.
// Channel 1 (unreliable sequenced) - entity snapshots, player input, ping/pong.
// Channel 2 (reliable ordered) - chat, block updates, events.
// PacketHeader is prepended to every payload for validation.
// All multi-byte fields are little-endian for cross-platform compatibility.

} // namespace PoorCraft

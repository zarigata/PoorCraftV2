#include "poorcraft/network/PacketType.h"

namespace PoorCraft {

std::string getPacketTypeName(PacketType type) {
    switch (type) {
        case PacketType::HANDSHAKE_REQUEST: return "HandshakeRequest";
        case PacketType::HANDSHAKE_RESPONSE: return "HandshakeResponse";
        case PacketType::PLAYER_INPUT: return "PlayerInput";
        case PacketType::ENTITY_SNAPSHOT: return "EntitySnapshot";
        case PacketType::CHUNK_DATA: return "ChunkData";
        case PacketType::CHUNK_REQUEST: return "ChunkRequest";
        case PacketType::PLAYER_JOIN: return "PlayerJoin";
        case PacketType::PLAYER_LEAVE: return "PlayerLeave";
        case PacketType::CHAT_MESSAGE: return "ChatMessage";
        case PacketType::DISCONNECT: return "Disconnect";
        case PacketType::PING: return "Ping";
        case PacketType::PONG: return "Pong";
        case PacketType::BLOCK_UPDATE: return "BlockUpdate";
        case PacketType::PLAYER_SPAWN: return "PlayerSpawn";
        default: return "Unknown";
    }
}

bool isReliablePacket(PacketType type) {
    switch (type) {
        case PacketType::HANDSHAKE_REQUEST:
        case PacketType::HANDSHAKE_RESPONSE:
        case PacketType::PLAYER_JOIN:
        case PacketType::PLAYER_LEAVE:
        case PacketType::CHUNK_DATA:
        case PacketType::CHAT_MESSAGE:
        case PacketType::DISCONNECT:
        case PacketType::BLOCK_UPDATE:
        case PacketType::PLAYER_SPAWN:
            return true;
        case PacketType::PLAYER_INPUT:
        case PacketType::ENTITY_SNAPSHOT:
        case PacketType::CHUNK_REQUEST:
        case PacketType::PING:
        case PacketType::PONG:
            return false;
        default:
            return true;
    }
}

} // namespace PoorCraft

#include "poorcraft/network/NetworkClient.h"

#include <algorithm>

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/network/NetworkEvents.h"
#include "poorcraft/world/World.h"

namespace PoorCraft {

namespace {
constexpr double INTERPOLATION_DELAY = 0.1;
}

NetworkClient::NetworkClient(const std::string& playerName)
    : m_Host(nullptr),
      m_ServerPeer(nullptr),
      m_State(ConnectionState::DISCONNECTED),
      m_PlayerName(playerName),
      m_LocalPlayerId(0),
      m_World(nullptr),
      m_EntityManager(nullptr),
      m_NextInputSequence(1) {}

NetworkClient::~NetworkClient() {
    disconnect("Client shutdown");
    if (m_Host) {
        enet_host_destroy(m_Host);
        m_Host = nullptr;
    }
}

bool NetworkClient::connect(const std::string& address, std::uint16_t port) {
    if (m_Host) {
        return false;
    }

    m_Host = enet_host_create(nullptr, 1, 3, 0, 0);
    if (!m_Host) {
        PC_ERROR("Failed to create ENet client host");
        return false;
    }

    ENetAddress addr;
    if (enet_address_set_host(&addr, address.c_str()) != 0) {
        PC_ERROR("Invalid server address: " + address);
        return false;
    }
    addr.port = port;

    m_ServerPeer = enet_host_connect(m_Host, &addr, 3, 0);
    if (!m_ServerPeer) {
        PC_ERROR("Failed to initiate connection to server");
        return false;
    }

    m_State = ConnectionState::CONNECTING;
    PC_INFO("Connecting to server " + address + ":" + std::to_string(port));
    return true;
}

void NetworkClient::disconnect(const std::string& reason) {
    if (!m_Host || !m_ServerPeer) {
        return;
    }

    if (m_State == ConnectionState::CONNECTED) {
        DisconnectPacket packet;
        packet.reason = reason;
        PacketWriter writer;
        packet.serialize(writer);

        NetworkPeer peer(m_ServerPeer);
        peer.sendPacket(PacketType::DISCONNECT, writer, 0);
    }

    enet_peer_disconnect(m_ServerPeer, 0);
    enet_host_flush(m_Host);
    m_State = ConnectionState::DISCONNECTED;
}

void NetworkClient::update(float deltaTime) {
    if (!m_Host) {
        return;
    }

    ENetEvent event;
    while (enet_host_service(m_Host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                handleConnect(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                handleReceive(event);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                handleDisconnect(event);
                break;
            default:
                break;
        }
    }
}

void NetworkClient::sendInput(const PlayerInputPacket& input) {
    if (m_State != ConnectionState::CONNECTED || !m_ServerPeer) {
        return;
    }

    PlayerInputPacket packet = input;
    packet.inputSequence = m_NextInputSequence++;

    PacketWriter writer;
    packet.serialize(writer);

    NetworkPeer peer(m_ServerPeer);
    peer.sendPacket(PacketType::PLAYER_INPUT, writer, 1);

    m_InputBuffer.push_back(packet);
    if (m_InputBuffer.size() > 256) {
        m_InputBuffer.pop_front();
    }
}

void NetworkClient::requestChunk(std::int32_t chunkX, std::int32_t chunkZ) {
    if (m_State != ConnectionState::CONNECTED || !m_ServerPeer) {
        return;
    }

    ChunkRequestPacket packet;
    packet.chunkX = chunkX;
    packet.chunkZ = chunkZ;

    PacketWriter writer;
    packet.serialize(writer);

    NetworkPeer peer(m_ServerPeer);
    peer.sendPacket(PacketType::CHUNK_REQUEST, writer, 0);
}

void NetworkClient::sendChatMessage(const std::string& message) {
    if (m_State != ConnectionState::CONNECTED || !m_ServerPeer) {
        return;
    }

    ChatMessagePacket packet;
    packet.senderId = m_LocalPlayerId;
    packet.message = message;
    packet.timestamp = 0;

    PacketWriter writer;
    packet.serialize(writer);

    NetworkPeer peer(m_ServerPeer);
    peer.sendPacket(PacketType::CHAT_MESSAGE, writer, 2);
}

ConnectionState NetworkClient::getConnectionState() const {
    return m_State;
}

std::uint32_t NetworkClient::getPing() const {
    if (!m_ServerPeer) {
        return 0;
    }
    return m_ServerPeer->roundTripTime;
}

void NetworkClient::setWorld(World* world) {
    m_World = world;
}

void NetworkClient::setEntityManager(EntityManager* entityManager) {
    m_EntityManager = entityManager;
}

void NetworkClient::setLocalPlayerId(EntityID id) {
    m_LocalPlayerId = id;
}

void NetworkClient::handleConnect(ENetEvent& event) {
    PC_INFO("Connected to server");
    m_State = ConnectionState::CONNECTED;

    HandshakeRequestPacket packet;
    packet.protocolVersion = 1;
    packet.playerName = m_PlayerName;
    packet.clientVersion = "0.1";

    PacketWriter writer;
    packet.serialize(writer);

    NetworkPeer peer(event.peer);
    peer.sendPacket(PacketType::HANDSHAKE_REQUEST, writer, 0);
}

void NetworkClient::handleReceive(ENetEvent& event) {
    PacketReader reader(event.packet->data, event.packet->dataLength);
    const PacketType type = static_cast<PacketType>(reader.readUInt8());

    switch (type) {
        case PacketType::HANDSHAKE_RESPONSE:
            processHandshakeResponse(reader);
            break;
        case PacketType::ENTITY_SNAPSHOT:
            processEntitySnapshot(reader);
            break;
        case PacketType::CHUNK_DATA:
            processChunkData(reader);
            break;
        case PacketType::PLAYER_JOIN:
            processPlayerJoin(reader);
            break;
        case PacketType::PLAYER_LEAVE:
            processPlayerLeave(reader);
            break;
        case PacketType::CHAT_MESSAGE:
            processChatMessage(reader);
            break;
        default:
            PC_WARN("Unhandled packet type on client: " + getPacketTypeName(type));
            break;
    }
}

void NetworkClient::handleDisconnect(ENetEvent& event) {
    (void)event;
    m_State = ConnectionState::DISCONNECTED;
    m_ServerPeer = nullptr;
    PC_INFO("Disconnected from server");
}

void NetworkClient::processHandshakeResponse(PacketReader& reader) {
    auto response = HandshakeResponsePacket::deserialize(reader);
    if (!response.accepted) {
        PC_ERROR("Handshake rejected: " + response.serverMessage);
        disconnect("Handshake rejected");
        return;
    }

    m_LocalPlayerId = response.playerId;
    PC_INFO("Handshake accepted, assigned player ID " + std::to_string(response.playerId));
}

void NetworkClient::processEntitySnapshot(PacketReader& reader) {
    auto snapshot = EntitySnapshotPacket::deserialize(reader);
    reconcileLocalPlayer(snapshot);

    m_SnapshotBuffer.push_back(snapshot);
    if (m_SnapshotBuffer.size() > 32) {
        m_SnapshotBuffer.erase(m_SnapshotBuffer.begin());
    }
}

void NetworkClient::processChunkData(PacketReader& reader) {
    auto chunkPacket = ChunkDataPacket::deserialize(reader);
    (void)chunkPacket;
}

void NetworkClient::processPlayerJoin(PacketReader& reader) {
    auto joinPacket = PlayerJoinPacket::deserialize(reader);
    PC_INFO("Player joined: " + joinPacket.playerName);
}

void NetworkClient::processPlayerLeave(PacketReader& reader) {
    auto leavePacket = PlayerLeavePacket::deserialize(reader);
    PC_INFO("Player left: " + std::to_string(leavePacket.playerId));
}

void NetworkClient::processChatMessage(PacketReader& reader) {
    auto chatPacket = ChatMessagePacket::deserialize(reader);
    PC_INFO("Chat: " + chatPacket.message);
}

void NetworkClient::reconcileLocalPlayer(const EntitySnapshotPacket& snapshot) {
    (void)snapshot;
}

} // namespace PoorCraft

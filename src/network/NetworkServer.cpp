#include "poorcraft/network/NetworkServer.h"

#include <algorithm>
#include <cmath>

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/network/ChunkCompression.h"
#include "poorcraft/network/NetworkEvents.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/World.h"

namespace PoorCraft {

namespace {
constexpr double SERVER_TICK_RATE = 60.0;
constexpr double SNAPSHOT_RATE = 20.0;
}

NetworkServer::NetworkServer(std::uint16_t port, std::size_t maxClients)
    : m_Host(nullptr),
      m_Port(port),
      m_MaxClients(maxClients),
      m_World(nullptr),
      m_EntityManager(nullptr),
      m_ServerTick(0),
      m_Accumulator(0.0),
      m_SnapshotAccumulator(0.0),
      m_SnapshotInterval(1.0 / SNAPSHOT_RATE) {}

NetworkServer::~NetworkServer() {
    shutdown();
}

bool NetworkServer::initialize() {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = m_Port;

    m_Host = enet_host_create(&address, m_MaxClients, 3, 0, 0);
    if (!m_Host) {
        PC_ERROR("Failed to create ENet host for server");
        return false;
    }

    PC_INFO("Network server listening on port " + std::to_string(m_Port));
    return true;
}

void NetworkServer::shutdown() {
    if (!m_Host) {
        return;
    }

    for (auto& client : m_Clients) {
        client.peer.disconnect();
    }

    enet_host_flush(m_Host);
    enet_host_destroy(m_Host);
    m_Host = nullptr;
    m_Clients.clear();

    PC_INFO("Network server shutdown");
}

void NetworkServer::update(float deltaTime) {
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

    m_Accumulator += deltaTime;
    m_SnapshotAccumulator += deltaTime;

    const double tickInterval = 1.0 / SERVER_TICK_RATE;
    while (m_Accumulator >= tickInterval) {
        ++m_ServerTick;
        m_Accumulator -= tickInterval;
    }

    if (m_SnapshotAccumulator >= m_SnapshotInterval) {
        sendEntitySnapshot();
        m_SnapshotAccumulator = 0.0;
    }

    for (auto& client : m_Clients) {
        updateChunkStreaming(client);
    }
}

std::size_t NetworkServer::getConnectedClientCount() const {
    return m_Clients.size();
}

std::uint32_t NetworkServer::getServerTick() const {
    return m_ServerTick;
}

void NetworkServer::setWorld(World* world) {
    m_World = world;
}

void NetworkServer::setEntityManager(EntityManager* entityManager) {
    m_EntityManager = entityManager;
}

void NetworkServer::handleConnect(ENetEvent& event) {
    PC_INFO("Peer connected: " + std::to_string(event.peer->incomingPeerID));
    NetworkPeer peer(event.peer);
    m_Clients.push_back({peer});
}

void NetworkServer::handleReceive(ENetEvent& event) {
    PacketReader reader(event.packet->data, event.packet->dataLength);
    const PacketType type = static_cast<PacketType>(reader.readUInt8());

    switch (type) {
        case PacketType::HANDSHAKE_REQUEST:
            processHandshake(findClientByPeer(event.peer)->peer, reader);
            break;
        case PacketType::PLAYER_INPUT: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                processPlayerInput(*client, reader);
            }
            break;
        }
        case PacketType::CHUNK_REQUEST: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                processChunkRequest(*client, reader);
            }
            break;
        }
        default:
            PC_WARN("Unhandled packet type: " + getPacketTypeName(type));
            break;
    }
}

void NetworkServer::handleDisconnect(ENetEvent& event) {
    ConnectedClient* client = findClientByPeer(event.peer);
    if (!client) {
        return;
    }

    PC_INFO("Peer disconnected: " + std::to_string(event.peer->incomingPeerID));

    if (m_EntityManager && client->playerId != 0) {
        m_EntityManager->destroyEntity(client->playerId);
    }

    m_Clients.erase(std::remove_if(m_Clients.begin(), m_Clients.end(), [&](const ConnectedClient& connected) {
        return connected.peer.getHandle() == event.peer;
    }), m_Clients.end());
}

void NetworkServer::processHandshake(NetworkPeer& peer, PacketReader& reader) {
    auto packet = HandshakeRequestPacket::deserialize(reader);
    PC_INFO("Received handshake from " + packet.playerName);

    ConnectedClient* client = findClientByPeer(peer.getHandle());
    if (!client) {
        return;
    }

    client->playerName = packet.playerName;

    HandshakeResponsePacket response;
    response.accepted = true;
    response.playerId = client->playerId;
    response.spawnPosition = {0.0f, 64.0f, 0.0f};
    response.worldSeed = 0;
    response.serverMessage = "Welcome";

    PacketWriter writer;
    response.serialize(writer);
    peer.sendPacket(PacketType::HANDSHAKE_RESPONSE, writer, 0);
}

void NetworkServer::processPlayerInput(ConnectedClient& client, PacketReader& reader) {
    auto packet = PlayerInputPacket::deserialize(reader);
    client.lastInputSequence = packet.inputSequence;
}

void NetworkServer::processChunkRequest(ConnectedClient& client, PacketReader& reader) {
    auto request = ChunkRequestPacket::deserialize(reader);
    sendChunkToClient(client, request.chunkX, request.chunkZ);
}

void NetworkServer::sendEntitySnapshot() {
    if (!m_EntityManager) {
        return;
    }

    EntitySnapshotPacket packet;
    packet.serverTick = m_ServerTick;
    packet.lastConsumedInputSeq = 0;

    PacketWriter writer;
    packet.serialize(writer);

    for (auto& client : m_Clients) {
        client.peer.sendPacket(PacketType::ENTITY_SNAPSHOT, writer, 1);
        client.lastSnapshotTick = m_ServerTick;
    }
}

void NetworkServer::sendChunkToClient(ConnectedClient& client, std::int32_t chunkX, std::int32_t chunkZ) {
    if (!m_World) {
        return;
    }

    Chunk* chunk = m_World->getChunk(chunkX, chunkZ);
    if (!chunk) {
        return;
    }

    ChunkDataPacket packet;
    packet.chunkX = chunkX;
    packet.chunkZ = chunkZ;
    packet.blockData = ChunkCompression::compressChunk(*chunk);

    PacketWriter writer;
    packet.serialize(writer);
    client.peer.sendPacket(PacketType::CHUNK_DATA, writer, 0);

    client.loadedChunks.insert(getChunkKey(chunkX, chunkZ));
}

void NetworkServer::updateChunkStreaming(ConnectedClient& client) {
    (void)client;
}

ConnectedClient* NetworkServer::findClientByPeer(ENetPeer* peer) {
    if (!peer) {
        return nullptr;
    }

    for (auto& client : m_Clients) {
        if (client.peer.getHandle() == peer) {
            return &client;
        }
    }
    return nullptr;
}

std::int64_t NetworkServer::getChunkKey(std::int32_t x, std::int32_t z) const {
    return (static_cast<std::int64_t>(x) << 32) | (static_cast<std::int64_t>(z) & 0xffffffffLL);
}

} // namespace PoorCraft

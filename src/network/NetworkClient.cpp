#include "poorcraft/network/NetworkClient.h"

#include <algorithm>
#include <cmath>

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/network/NetworkEvents.h"
#include "poorcraft/network/ChunkCompression.h"
#include "poorcraft/entity/components/NetworkIdentity.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/world/World.h"
#include "poorcraft/world/ChunkManager.h"

#include "poorcraft/core/Config.h"

namespace PoorCraft {

namespace {
constexpr double INTERPOLATION_DELAY = 0.1;
constexpr double MILLISECONDS_TO_SECONDS = 0.001;
}

NetworkClient::NetworkClient(const std::string& playerName)
    : m_Host(nullptr),
      m_ServerPeer(nullptr),
      m_State(ConnectionState::DISCONNECTED),
      m_PlayerName(playerName),
      m_LocalPlayerId(0),
      m_World(nullptr),
      m_EntityManager(nullptr),
      m_NextInputSequence(1),
      m_LastSequenceReceived(0),
      m_LastPacketTimestamp(0),
      m_ServerTimeOffset(0.0),
      m_LastPingTime(0.0),
      m_PingInterval(1.0),
      m_ServerAddress(""),
      m_ServerPort(0) {}

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
    m_ServerAddress = address;
    m_ServerPort = port;
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

    const double currentTime = enet_time_get() / 1000.0;
    if (m_State == ConnectionState::CONNECTED && m_ServerPeer) {
        if (currentTime - m_LastPingTime >= m_PingInterval) {
            PingPacket ping{};
            ping.clientTime = static_cast<std::uint32_t>(currentTime * 1000.0);

            PacketWriter writer;
            ping.serialize(writer);

            NetworkPeer peer(m_ServerPeer);
            peer.sendPacket(PacketType::PING, writer, 1);
            m_LastPingTime = currentTime;
        }
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

    if (m_State == ConnectionState::CONNECTED && m_EntityManager) {
        const auto& config = poorcraft::Config::get_instance();
        const int interpolationDelayMs = config.get_int(poorcraft::Config::NetworkConfig::INTERPOLATION_DELAY_KEY, 100);
        const double clientTimeMs = static_cast<double>(enet_time_get());
        const double renderTimeMs = (clientTimeMs + m_ServerTimeOffset) - static_cast<double>(interpolationDelayMs);
        updateRemoteEntities(renderTimeMs);
        cleanupChunkAssemblies(clientTimeMs);
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

    EventBus::getInstance().publish(ConnectionEstablishedEvent(m_ServerAddress, m_ServerPort, 0));
}

void NetworkClient::handleReceive(ENetEvent& event) {
    if (!event.packet || event.packet->dataLength < PacketHeader::SIZE) {
        PC_WARN("Received undersized packet on client");
        return;
    }

    PacketReader headerReader(event.packet->data, event.packet->dataLength);
    PacketHeader header{};
    header.type = headerReader.readUInt8();
    header.size = headerReader.readUInt16();
    header.sequence = headerReader.readUInt32();
    header.timestamp = headerReader.readUInt32();

    const std::size_t payloadAvailable = event.packet->dataLength - PacketHeader::SIZE;
    if (header.size > payloadAvailable) {
        PC_WARN("Packet payload truncated on client for type: " + getPacketTypeName(header.getPacketType()));
        return;
    }

    PacketReader payloadReader(event.packet->data + PacketHeader::SIZE, header.size);
    const PacketType type = header.getPacketType();

    m_LastSequenceReceived = header.sequence;
    m_LastPacketTimestamp = header.timestamp;

    switch (type) {
        case PacketType::HANDSHAKE_RESPONSE:
            processHandshakeResponse(payloadReader);
            break;
        case PacketType::ENTITY_SNAPSHOT:
            processEntitySnapshot(payloadReader);
            break;
        case PacketType::CHUNK_DATA:
            processChunkData(payloadReader);
            break;
        case PacketType::PLAYER_JOIN:
            processPlayerJoin(payloadReader);
            break;
        case PacketType::PLAYER_LEAVE:
            processPlayerLeave(payloadReader);
            break;
        case PacketType::CHAT_MESSAGE:
            processChatMessage(payloadReader);
            break;
        case PacketType::PING: {
            PingPacket ping = PingPacket::deserialize(payloadReader);
            PacketWriter writer;
            writer.writeUInt32(ping.clientTime);
            writer.writeUInt32(static_cast<std::uint32_t>(enet_time_get()));
            NetworkPeer peer(m_ServerPeer);
            peer.sendPacket(PacketType::PONG, writer, 1);
            break;
        }
        case PacketType::PONG: {
            PongPacket pong = PongPacket::deserialize(payloadReader);
            const double clientTime = enet_time_get();
            const double rtt = std::max(0.0, clientTime - static_cast<double>(pong.clientTime));
            const double estimatedServerTime = static_cast<double>(pong.serverTime) - rtt * 0.5;
            const double newOffset = estimatedServerTime - clientTime;
            m_ServerTimeOffset = (m_ServerTimeOffset * 0.9) + (newOffset * 0.1);
            break;
        }
        default:
            PC_WARN("Unhandled packet type on client: " + getPacketTypeName(type));
            break;
    }
}

void NetworkClient::handleDisconnect(ENetEvent& event) {
    (void)event;
    EventBus::getInstance().publish(ConnectionLostEvent("Disconnected", false));
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

    EventBus::getInstance().publish(ConnectionEstablishedEvent(m_ServerAddress, m_ServerPort, response.playerId));
}

void NetworkClient::processEntitySnapshot(PacketReader& reader) {
    if (!m_EntityManager) {
        return;
    }

    EntitySnapshotPacket snapshot = EntitySnapshotPacket::deserialize(reader);

    for (const auto& state : snapshot.entities) {
        if (state.entityId == m_LocalPlayerId) {
            reconcileLocalPlayer(snapshot);
            continue;
        }

        Entity* entity = m_EntityManager->getEntity(state.entityId);
        if (!entity) {
            entity = &m_EntityManager->createEntity("RemoteEntity:" + std::to_string(state.entityId));
        }

        auto& transform = entity->addComponent<Transform>();
        auto& netIdentity = entity->addComponent<NetworkIdentity>();
        if (netIdentity.getNetworkId() != state.entityId) {
            netIdentity.setNetworkId(state.entityId);
        }
        netIdentity.setLocalPlayer(false);
        netIdentity.setServerControlled(true);

        NetworkSnapshot snap{};
        snap.tick = snapshot.serverTick;
        snap.position = state.position;
        snap.velocity = state.velocity;
        snap.rotation = state.rotation;
        snap.animationState = state.animationState;
        netIdentity.addSnapshot(snap);
        (void)transform;
    }

    m_InputBuffer.erase(std::remove_if(m_InputBuffer.begin(), m_InputBuffer.end(), [&](const PlayerInputPacket& input) {
        return input.inputSequence <= snapshot.lastConsumedInputSeq;
    }), m_InputBuffer.end());
}

void NetworkClient::processChunkData(PacketReader& reader) {
    ChunkDataPacket fragment = ChunkDataPacket::deserialize(reader);
    const double currentTimeMs = static_cast<double>(enet_time_get());
    const std::uint64_t key = getChunkKey(fragment.chunkX, fragment.chunkZ);

    auto& buffer = m_PendingChunkFragments[key];
    buffer.lastUpdateTime = currentTimeMs;
    buffer.fragments.emplace(fragment.fragmentId, fragment.blockData);
    buffer.totalSize += fragment.blockData.size();
    if (fragment.isLast) {
        buffer.lastReceived = true;
        buffer.lastFragmentId = fragment.fragmentId;
    }

    bool complete = buffer.lastReceived && buffer.fragments.size() == static_cast<std::size_t>(buffer.lastFragmentId + 1);
    if (!complete) {
        return;
    }

    std::vector<std::uint8_t> assembled;
    assembled.reserve(buffer.totalSize);
    for (std::uint16_t i = 0; i <= buffer.lastFragmentId; ++i) {
        const auto it = buffer.fragments.find(i);
        if (it == buffer.fragments.end()) {
            return;
        }
        assembled.insert(assembled.end(), it->second.begin(), it->second.end());
    }

    processCompleteChunk(fragment.chunkX, fragment.chunkZ, assembled);
    m_PendingChunkFragments.erase(key);
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
    if (!m_EntityManager) {
        return;
    }

    Entity* entity = m_EntityManager->getEntity(m_LocalPlayerId);
    if (!entity) {
        return;
    }

    auto* transform = entity->getComponent<Transform>();
    auto* netIdentity = entity->getComponent<NetworkIdentity>();
    if (!transform) {
        transform = &entity->addComponent<Transform>();
    }
    if (netIdentity) {
        netIdentity->setLocalPlayer(true);
        netIdentity->setNetworkId(m_LocalPlayerId);
    }

    const auto it = std::find_if(snapshot.entities.begin(), snapshot.entities.end(), [&](const EntityStateData& state) {
        return state.entityId == m_LocalPlayerId;
    });
    if (it == snapshot.entities.end()) {
        return;
    }

    const glm::vec3 predictedPosition = transform->getPosition();
    const glm::vec3& serverPosition = it->position;
    const glm::vec3 error = serverPosition - predictedPosition;
    const float errorMagnitude = glm::length(error);

    const auto& config = poorcraft::Config::get_instance();
    const float threshold = config.get_float(poorcraft::Config::NetworkConfig::PREDICTION_ERROR_THRESHOLD_KEY, 0.5f);

    if (errorMagnitude > threshold) {
        transform->setPosition(serverPosition);
    } else {
        transform->setPosition(predictedPosition + error * 0.1f);
    }

    m_InputBuffer.erase(std::remove_if(m_InputBuffer.begin(), m_InputBuffer.end(), [&](const PlayerInputPacket& input) {
        return input.inputSequence <= snapshot.lastConsumedInputSeq;
    }), m_InputBuffer.end());
}

void NetworkClient::addRemoteSnapshot(const EntityStateData& state, std::uint32_t serverTick) {
    if (!m_EntityManager) {
        return;
    }

    Entity* entity = m_EntityManager->getEntity(state.entityId);
    if (!entity) {
        entity = &m_EntityManager->createEntity("RemoteEntity:" + std::to_string(state.entityId));
    }

    auto& transform = entity->addComponent<Transform>();
    auto& netIdentity = entity->addComponent<NetworkIdentity>();
    netIdentity.setNetworkId(state.entityId);
    netIdentity.setLocalPlayer(false);
    netIdentity.setServerControlled(true);

    NetworkSnapshot snapshot{};
    snapshot.tick = serverTick;
    snapshot.position = state.position;
    snapshot.velocity = state.velocity;
    snapshot.rotation = state.rotation;
    snapshot.animationState = state.animationState;
    netIdentity.addSnapshot(snapshot);
    (void)transform;
}

void NetworkClient::updateRemoteEntities(double renderTimeMs) {
    if (!m_EntityManager) {
        return;
    }

    const auto entities = m_EntityManager->getEntitiesWith<NetworkIdentity>();
    for (Entity* entity : entities) {
        if (!entity) {
            continue;
        }

        auto* netIdentity = entity->getComponent<NetworkIdentity>();
        auto* transform = entity->getComponent<Transform>();
        if (!netIdentity || !transform) {
            continue;
        }

        if (netIdentity->isLocalPlayer()) {
            continue;
        }

        NetworkSnapshot interpolated;
        if (netIdentity->getInterpolatedState(renderTimeMs * MILLISECONDS_TO_SECONDS, interpolated)) {
            transform->setPosition(interpolated.position);
            transform->setRotation(interpolated.rotation);
        }
    }
}

void NetworkClient::processCompleteChunk(std::int32_t chunkX, std::int32_t chunkZ, const std::vector<std::uint8_t>& data) {
    if (!m_World) {
        return;
    }

    ChunkManager& chunkManager = m_World->getChunkManager();
    ChunkCoord coord(chunkX, chunkZ);
    Chunk& chunk = chunkManager.getOrCreateChunk(coord);

    if (!ChunkCompression::decompressChunk(data, chunk)) {
        PC_WARN("Failed to decompress chunk (" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")");
        return;
    }

    chunk.setDirty(true);
    EventBus::getInstance().publish(ChunkReceivedEvent(coord, chunk.getBlockCount()));
}

std::uint64_t NetworkClient::getChunkKey(std::int32_t chunkX, std::int32_t chunkZ) const {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(chunkX)) << 32) |
           static_cast<std::uint32_t>(chunkZ);
}

void NetworkClient::cleanupChunkAssemblies(double currentTimeMs) {
    constexpr double EXPIRATION_MS = 5000.0;
    for (auto it = m_PendingChunkFragments.begin(); it != m_PendingChunkFragments.end();) {
        if (currentTimeMs - it->second.lastUpdateTime > EXPIRATION_MS) {
            it = m_PendingChunkFragments.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace PoorCraft

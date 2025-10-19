#include "poorcraft/network/NetworkServer.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/modding/ModManager.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/entity/components/AnimationController.h"
#include "poorcraft/entity/components/NetworkIdentity.h"
#include "poorcraft/entity/components/PlayerController.h"
#include "poorcraft/entity/components/Renderable.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/network/PacketSerializer.h"
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
      m_ModManager(nullptr),
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
    
    // Initialize mod system (server-only)
    if (m_ModManager) {
        std::string modsDir = Platform::join_path(Platform::get_executable_directory(), "mods");
        m_ModManager->initialize(modsDir);
        m_ModManager->loadMods();
        PC_INFO("Loaded {} mods", m_ModManager->getLoadedMods().size());
    }
    
    return true;
}

void NetworkServer::shutdown() {
    if (!m_Host) {
        return;
    }
    
    // Shutdown mods before server
    if (m_ModManager) {
        m_ModManager->shutdown();
    }

    for (auto& client : m_Clients) {
        client.peer.disconnect();
    }

    enet_host_flush(m_Host);
    enet_host_destroy(m_Host);
    m_Host = nullptr;
    m_Clients.clear();

    EventBus::getInstance().publish(ServerStoppedEvent("Server shutdown"));
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
    
    // Update mods
    if (m_ModManager) {
        m_ModManager->updateMods(deltaTime);
        
        // Check for hot-reload (development feature)
        #ifdef POORCRAFT_DEBUG
        static double hotReloadTimer = 0.0;
        hotReloadTimer += deltaTime;
        if (hotReloadTimer >= 1.0) {  // Check every second
            m_ModManager->checkForModifications();
            hotReloadTimer = 0.0;
        }
        #endif
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

void NetworkServer::setModManager(ModManager* modManager) {
    m_ModManager = modManager;
}

void NetworkServer::handleConnect(ENetEvent& event) {
    PC_INFO("Peer connected: " + std::to_string(event.peer->incomingPeerID));
    NetworkPeer peer(event.peer);
    m_Clients.push_back({peer});
}

void NetworkServer::handleReceive(ENetEvent& event) {
    if (event.packet->dataLength < PacketHeader::SIZE) {
        PC_WARN("Received undersized packet on server");
        return;
    }

    PacketReader headerReader(event.packet->data, event.packet->dataLength);
    PacketHeader header{};
    header.type = headerReader.readUInt8();
    header.size = headerReader.readUInt16();
    header.sequence = headerReader.readUInt32();
    header.timestamp = headerReader.readUInt32();

    const PacketType type = header.getPacketType();
    const std::size_t payloadAvailable = event.packet->dataLength - PacketHeader::SIZE;
    if (payloadAvailable < header.size) {
        PC_WARN("Packet payload truncated for type: " + getPacketTypeName(type));
        return;
    }

    PacketReader payloadReader(event.packet->data + PacketHeader::SIZE, header.size);

    switch (type) {
        case PacketType::HANDSHAKE_REQUEST: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                processHandshake(client->peer, payloadReader);
                client->lastSequenceReceived = header.sequence;
                client->lastPacketTimestamp = header.timestamp;
            }
            break;
        }
        case PacketType::PLAYER_INPUT: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                processPlayerInput(*client, payloadReader);
                client->lastSequenceReceived = header.sequence;
                client->lastPacketTimestamp = header.timestamp;
            }
            break;
        }
        case PacketType::CHUNK_REQUEST: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                processChunkRequest(*client, payloadReader);
                client->lastSequenceReceived = header.sequence;
                client->lastPacketTimestamp = header.timestamp;
            }
            break;
        }
        case PacketType::PING: {
            ConnectedClient* client = findClientByPeer(event.peer);
            if (client) {
                PingPacket ping = PingPacket::deserialize(payloadReader);
                PongPacket pong;
                pong.clientTime = ping.clientTime;
                pong.serverTime = static_cast<std::uint32_t>(enet_time_get());

                PacketWriter writer;
                pong.serialize(writer);
                client->peer.sendPacket(PacketType::PONG, writer, 1);
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
        if (Entity* player = m_EntityManager->getEntity(client->playerId)) {
            std::string playerName = player->getName();
            EventBus::getInstance().publish(PlayerLeftEvent(client->playerId, playerName, "Disconnected"));
        }
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

    if (!m_EntityManager || packet.playerName.empty()) {
        HandshakeResponsePacket response;
        response.accepted = false;
        response.serverMessage = "Invalid handshake";

        PacketWriter writer;
        response.serialize(writer);
        peer.sendPacket(PacketType::HANDSHAKE_RESPONSE, writer, 0);
        return;
    }

    Entity& entity = m_EntityManager->createEntity("Player:" + packet.playerName);
    client->playerId = entity.getId();
    client->playerName = packet.playerName;

    auto& transform = entity.addComponent<Transform>();
    glm::vec3 spawnPosition(0.0f, 64.0f, 0.0f);
    transform.setPosition(spawnPosition);
    transform.updatePrevious();

    auto physicsWorld = std::shared_ptr<PhysicsWorld>(nullptr);
    auto cameraPtr = std::shared_ptr<Camera>(nullptr);
    entity.addComponent<PlayerController>(physicsWorld, cameraPtr);
    entity.addComponent<Renderable>(nullptr, nullptr, std::vector<Renderable::Section>());
    entity.addComponent<AnimationController>();

    auto& networkIdentity = entity.addComponent<NetworkIdentity>();
    networkIdentity.setNetworkId(entity.getId());
    networkIdentity.setOwnerId(entity.getId());
    networkIdentity.setServerControlled(true);

    HandshakeResponsePacket response;
    response.accepted = true;
    response.playerId = entity.getId();
    response.spawnPosition = spawnPosition;
    response.worldSeed = 0;
    response.serverMessage = "Welcome";

    PacketWriter writer;
    response.serialize(writer);
    peer.sendPacket(PacketType::HANDSHAKE_RESPONSE, writer, 0);

    PlayerJoinPacket joinPacket;
    joinPacket.playerId = entity.getId();
    joinPacket.playerName = packet.playerName;
    joinPacket.spawnPosition = spawnPosition;

    PacketWriter joinWriter;
    joinPacket.serialize(joinWriter);

    for (auto& otherClient : m_Clients) {
        if (otherClient.peer.getHandle() != peer.getHandle()) {
            otherClient.peer.sendPacket(PacketType::PLAYER_JOIN, joinWriter, 0);
        }
    }

    EventBus::getInstance().publish(PlayerJoinedEvent(entity.getId(), packet.playerName, spawnPosition));
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

    std::vector<const Entity*> entities = m_EntityManager->getEntitiesWith<NetworkIdentity>();
    if (entities.empty()) {
        return;
    }

    for (auto& client : m_Clients) {
        EntitySnapshotPacket packet;
        packet.serverTick = m_ServerTick;
        packet.lastConsumedInputSeq = client.lastInputSequence;

        for (const Entity* entity : entities) {
            if (!entity) {
                continue;
            }
            const auto* netIdentity = entity->getComponent<NetworkIdentity>();
            const auto* transform = entity->getComponent<Transform>();
            if (!netIdentity || !transform) {
                continue;
            }

            EntityStateData state;
            state.entityId = netIdentity->getNetworkId();
            state.position = transform->getPosition();
            state.velocity = glm::vec3(0.0f);
            state.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            state.animationState = 0;
            state.stateFlags = 0;
            packet.entities.push_back(state);
        }

        PacketWriter writer;
        packet.serialize(writer);
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

    const auto& config = poorcraft::Config::get_instance();
    const int maxPacketSize = config.get_int(poorcraft::Config::NetworkConfig::MAX_PACKET_SIZE_KEY, 1200);
    const std::size_t headerSize = PacketHeader::SIZE;
    const std::size_t maxPayload = maxPacketSize > 0 && maxPacketSize > static_cast<int>(headerSize) ?
        static_cast<std::size_t>(maxPacketSize) - headerSize : packet.blockData.size();

    std::size_t offset = 0;
    std::uint16_t fragmentId = 0;
    while (offset < packet.blockData.size()) {
        const std::size_t remaining = packet.blockData.size() - offset;
        const std::size_t fragmentSize = std::min(remaining, maxPayload);

        ChunkDataPacket fragmentPacket;
        fragmentPacket.chunkX = chunkX;
        fragmentPacket.chunkZ = chunkZ;
        fragmentPacket.fragmentId = fragmentId;
        fragmentPacket.isLast = (offset + fragmentSize) >= packet.blockData.size();
        fragmentPacket.blockData.assign(packet.blockData.begin() + static_cast<std::ptrdiff_t>(offset),
                                        packet.blockData.begin() + static_cast<std::ptrdiff_t>(offset + fragmentSize));

        PacketWriter writer;
        fragmentPacket.serialize(writer);
        client.peer.sendPacket(PacketType::CHUNK_DATA, writer, 0);

        offset += fragmentSize;
        ++fragmentId;
    }

    client.loadedChunks.insert(getChunkKey(chunkX, chunkZ));
}

void NetworkServer::updateChunkStreaming(ConnectedClient& client) {
    if (!m_EntityManager || !m_World) {
        return;
    }

    if (client.playerId == 0) {
        return;
    }

    Entity* entity = m_EntityManager->getEntity(client.playerId);
    if (!entity) {
        return;
    }

    const Transform* transform = entity->getComponent<Transform>();
    if (!transform) {
        return;
    }

    const glm::vec3 position = transform->getPosition();
    const int renderDistance = poorcraft::Config::get_instance().get_int(poorcraft::Config::GameplayConfig::RENDER_DISTANCE_KEY, 8);

    std::unordered_set<std::int64_t> desiredChunks;
    const int radius = renderDistance;
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            const int chunkX = static_cast<int>(std::floor(position.x / Chunk::CHUNK_SIZE_X)) + dx;
            const int chunkZ = static_cast<int>(std::floor(position.z / Chunk::CHUNK_SIZE_Z)) + dz;
            desiredChunks.insert(getChunkKey(chunkX, chunkZ));
        }
    }

    for (const auto& key : desiredChunks) {
        if (client.loadedChunks.count(key) == 0) {
            const int chunkX = static_cast<int>(key >> 32);
            const int chunkZ = static_cast<int>(key & 0xffffffffLL);
            sendChunkToClient(client, chunkX, chunkZ);
        }
    }

    std::vector<std::int64_t> chunksToUnload;
    for (const auto& key : client.loadedChunks) {
        if (desiredChunks.count(key) == 0) {
            chunksToUnload.push_back(key);
        }
    }

    for (const auto& key : chunksToUnload) {
        client.loadedChunks.erase(key);
    }
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

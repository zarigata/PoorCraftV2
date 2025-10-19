#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "poorcraft/entity/Entity.h"
#include "poorcraft/network/NetworkPackets.h"
#include "poorcraft/network/NetworkPeer.h"

struct _ENetHost;
typedef struct _ENetHost ENetHost;

typedef unsigned int enet_uint32;

typedef unsigned int enet_uint;

namespace PoorCraft {

class World;
class EntityManager;
class Chunk;
class ModManager;

struct ChunkCoord;

struct ConnectedClient {
    NetworkPeer peer;
    EntityID playerId = 0;
    std::string playerName;
    std::uint32_t lastInputSequence = 0;
    std::uint32_t lastSnapshotTick = 0;
    std::unordered_set<std::int64_t> loadedChunks;
    double connectionTime = 0.0;
    std::uint32_t lastSequenceReceived = 0;
    std::uint32_t lastPacketTimestamp = 0;
};

class NetworkServer {
public:
    NetworkServer(std::uint16_t port, std::size_t maxClients);
    ~NetworkServer();

    bool initialize();
    void shutdown();

    void update(float deltaTime);

    std::size_t getConnectedClientCount() const;
    std::uint32_t getServerTick() const;

    void setWorld(World* world);
    void setEntityManager(EntityManager* entityManager);
    void setModManager(ModManager* modManager);

private:
    void handleConnect(struct _ENetEvent& event);
    void handleReceive(struct _ENetEvent& event);
    void handleDisconnect(struct _ENetEvent& event);

    void processHandshake(NetworkPeer& peer, PacketReader& reader);
    void processPlayerInput(ConnectedClient& client, PacketReader& reader);
    void processChunkRequest(ConnectedClient& client, PacketReader& reader);

    void sendEntitySnapshot();
    void sendChunkToClient(ConnectedClient& client, std::int32_t chunkX, std::int32_t chunkZ);
    void updateChunkStreaming(ConnectedClient& client);

    ConnectedClient* findClientByPeer(ENetPeer* peer);

    std::int64_t getChunkKey(std::int32_t x, std::int32_t z) const;

    ENetHost* m_Host;
    std::uint16_t m_Port;
    std::size_t m_MaxClients;

    std::vector<ConnectedClient> m_Clients;

    World* m_World;
    EntityManager* m_EntityManager;
    ModManager* m_ModManager;

    std::uint32_t m_ServerTick;
    double m_Accumulator;
    double m_SnapshotAccumulator;
    double m_SnapshotInterval;
};

} // namespace PoorCraft

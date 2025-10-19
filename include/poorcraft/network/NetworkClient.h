#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
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

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};

class NetworkClient {
public:
    explicit NetworkClient(const std::string& playerName);
    ~NetworkClient();

    bool connect(const std::string& address, std::uint16_t port);
    void disconnect(const std::string& reason);

    void update(float deltaTime);

    void sendInput(const PlayerInputPacket& input);
    void requestChunk(std::int32_t chunkX, std::int32_t chunkZ);
    void sendChatMessage(const std::string& message);

    ConnectionState getConnectionState() const;
    std::uint32_t getPing() const;

    void setWorld(World* world);
    void setEntityManager(EntityManager* entityManager);
    void setLocalPlayerId(EntityID id);

private:
    void handleConnect(struct _ENetEvent& event);
    void handleReceive(struct _ENetEvent& event);
    void handleDisconnect(struct _ENetEvent& event);

    void processHandshakeResponse(PacketReader& reader);
    void processEntitySnapshot(PacketReader& reader);
    void processChunkData(PacketReader& reader);
    void processPlayerJoin(PacketReader& reader);
    void processPlayerLeave(PacketReader& reader);
    void processChatMessage(PacketReader& reader);

    void reconcileLocalPlayer(const EntitySnapshotPacket& snapshot);

    ENetHost* m_Host;
    ENetPeer* m_ServerPeer;

    ConnectionState m_State;

    std::string m_PlayerName;
    EntityID m_LocalPlayerId;

    World* m_World;
    EntityManager* m_EntityManager;

    std::deque<PlayerInputPacket> m_InputBuffer;
    std::vector<EntitySnapshotPacket> m_SnapshotBuffer;

    std::uint32_t m_NextInputSequence;
};

} // namespace PoorCraft

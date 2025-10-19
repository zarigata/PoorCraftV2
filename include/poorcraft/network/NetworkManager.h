#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "poorcraft/network/NetworkClient.h"
#include "poorcraft/network/NetworkServer.h"

namespace PoorCraft {

enum class NetworkMode {
    NONE,
    CLIENT,
    DEDICATED_SERVER,
    INTEGRATED_SERVER
};

class NetworkManager {
public:
    static NetworkManager& getInstance();

    void initialize(NetworkMode mode);
    void shutdown();

    bool startServer(std::uint16_t port, std::size_t maxPlayers);
    bool startIntegratedServer(std::uint16_t port, std::size_t maxPlayers);
    bool connectToServer(const std::string& address, std::uint16_t port, const std::string& playerName);

    void disconnect();
    void update(float deltaTime);

    NetworkMode getMode() const;
    bool isInitialized() const;
    bool isServer() const;
    bool isClient() const;

    NetworkServer* getServer();
    NetworkClient* getClient();

    void setWorld(class World* world);
    void setEntityManager(class EntityManager* entityManager);

private:
    NetworkManager();

    NetworkMode m_Mode;
    std::unique_ptr<NetworkServer> m_Server;
    std::unique_ptr<NetworkClient> m_Client;
    bool m_EnetInitialized;
    bool m_IsInitialized;
};

} // namespace PoorCraft

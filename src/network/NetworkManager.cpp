#include "poorcraft/network/NetworkManager.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/network/NetworkEvents.h"

#include <enet/enet.h>

namespace PoorCraft {

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager()
    : m_Mode(NetworkMode::NONE),
      m_EnetInitialized(false),
      m_IsInitialized(false) {}

void NetworkManager::initialize(NetworkMode mode) {
    if (m_IsInitialized) {
        PC_WARN("NetworkManager already initialized");
        return;
    }

    if (!m_EnetInitialized) {
        if (enet_initialize() != 0) {
            PC_FATAL("Failed to initialize ENet");
            return;
        }
        m_EnetInitialized = true;
    }

    m_Mode = mode;
    m_IsInitialized = true;
    PC_INFO("NetworkManager initialized");
}

void NetworkManager::shutdown() {
    if (m_Client) {
        m_Client->disconnect("Shutdown");
        m_Client.reset();
    }

    if (m_Server) {
        m_Server->shutdown();
        m_Server.reset();
    }

    if (m_Server) {
        EventBus::getInstance().publish(ServerStoppedEvent("NetworkManager shutdown"));
    }

    m_Mode = NetworkMode::NONE;
    m_IsInitialized = false;
    if (m_EnetInitialized) {
        enet_deinitialize();
        m_EnetInitialized = false;
    }
    PC_INFO("NetworkManager shutdown");
}

bool NetworkManager::startServer(std::uint16_t port, std::size_t maxPlayers) {
    if (!m_IsInitialized) {
        PC_ERROR("NetworkManager must be initialized before starting server");
        return false;
    }

    if (m_Server) {
        PC_ERROR("Server already running");
        return false;
    }

    m_Server = std::make_unique<NetworkServer>(port, maxPlayers);
    if (!m_Server->initialize()) {
        m_Server.reset();
        return false;
    }

    m_Mode = NetworkMode::DEDICATED_SERVER;
    EventBus::getInstance().publish(ServerStartedEvent(port, maxPlayers));
    return true;
}

bool NetworkManager::startIntegratedServer(std::uint16_t port, std::size_t maxPlayers) {
    if (!m_IsInitialized) {
        PC_ERROR("NetworkManager must be initialized before starting integrated server");
        return false;
    }

    if (!startServer(port, maxPlayers)) {
        return false;
    }

    m_Client = std::make_unique<NetworkClient>("Host");
    if (!m_Client->connect("localhost", port)) {
        m_Client.reset();
        return false;
    }

    m_Mode = NetworkMode::INTEGRATED_SERVER;
    return true;
}

bool NetworkManager::connectToServer(const std::string& address, std::uint16_t port, const std::string& playerName) {
    if (!m_IsInitialized) {
        PC_ERROR("NetworkManager must be initialized before connecting");
        return false;
    }

    if (m_Client) {
        PC_ERROR("Client already connected");
        return false;
    }

    m_Client = std::make_unique<NetworkClient>(playerName);
    if (!m_Client->connect(address, port)) {
        m_Client.reset();
        return false;
    }

    m_Mode = NetworkMode::CLIENT;
    return true;
}

void NetworkManager::disconnect() {
    if (m_Client) {
        m_Client->disconnect("Disconnected by user");
        m_Client.reset();
    }

    if (m_Mode == NetworkMode::DEDICATED_SERVER || m_Mode == NetworkMode::INTEGRATED_SERVER) {
        if (m_Server) {
            m_Server->shutdown();
            m_Server.reset();
        }
    }

    m_Mode = NetworkMode::NONE;
}

void NetworkManager::update(float deltaTime) {
    if (m_Server) {
        m_Server->update(deltaTime);
    }

    if (m_Client) {
        m_Client->update(deltaTime);
    }
}

NetworkMode NetworkManager::getMode() const {
    return m_Mode;
}

bool NetworkManager::isInitialized() const {
    return m_IsInitialized;
}

bool NetworkManager::isServer() const {
    return m_Mode == NetworkMode::DEDICATED_SERVER || m_Mode == NetworkMode::INTEGRATED_SERVER;
}

bool NetworkManager::isClient() const {
    return m_Mode == NetworkMode::CLIENT || m_Mode == NetworkMode::INTEGRATED_SERVER;
}

NetworkServer* NetworkManager::getServer() {
    return m_Server.get();
}

NetworkClient* NetworkManager::getClient() {
    return m_Client.get();
}

void NetworkManager::setWorld(World* world) {
    if (m_Server) {
        m_Server->setWorld(world);
    }
    if (m_Client) {
        m_Client->setWorld(world);
    }
}

void NetworkManager::setEntityManager(EntityManager* entityManager) {
    if (m_Server) {
        m_Server->setEntityManager(entityManager);
    }
    if (m_Client) {
        m_Client->setEntityManager(entityManager);
    }
}

} // namespace PoorCraft

#include "poorcraft/network/NetworkManager.h"

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager()
    : m_Mode(NetworkMode::NONE) {}

void NetworkManager::initialize(NetworkMode mode) {
    m_Mode = mode;
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

    m_Mode = NetworkMode::NONE;
    PC_INFO("NetworkManager shutdown");
}

bool NetworkManager::startServer(std::uint16_t port, std::size_t maxPlayers) {
    if (m_Mode != NetworkMode::NONE) {
        PC_ERROR("Network already initialized");
        return false;
    }

    m_Server = std::make_unique<NetworkServer>(port, maxPlayers);
    if (!m_Server->initialize()) {
        m_Server.reset();
        return false;
    }

    m_Mode = NetworkMode::DEDICATED_SERVER;
    return true;
}

bool NetworkManager::startIntegratedServer(std::uint16_t port, std::size_t maxPlayers) {
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
    if (m_Mode != NetworkMode::NONE) {
        PC_ERROR("Network already initialized");
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

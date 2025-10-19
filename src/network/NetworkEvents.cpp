#include "poorcraft/network/NetworkEvents.h"

#include <sstream>

namespace PoorCraft {

PlayerJoinedEvent::PlayerJoinedEvent(EntityID playerId, std::string name, const glm::vec3& position)
    : m_PlayerId(playerId),
      m_PlayerName(std::move(name)),
      m_Position(position) {}

EntityID PlayerJoinedEvent::getPlayerId() const {
    return m_PlayerId;
}

const std::string& PlayerJoinedEvent::getPlayerName() const {
    return m_PlayerName;
}

const glm::vec3& PlayerJoinedEvent::getPosition() const {
    return m_Position;
}

std::string PlayerJoinedEvent::toString() const {
    std::ostringstream oss;
    oss << "PlayerJoinedEvent: " << m_PlayerName << " (" << m_PlayerId << ") at ("
        << m_Position.x << ", " << m_Position.y << ", " << m_Position.z << ")";
    return oss.str();
}

PlayerLeftEvent::PlayerLeftEvent(EntityID playerId, std::string name, std::string reason)
    : m_PlayerId(playerId),
      m_PlayerName(std::move(name)),
      m_Reason(std::move(reason)) {}

EntityID PlayerLeftEvent::getPlayerId() const {
    return m_PlayerId;
}

const std::string& PlayerLeftEvent::getPlayerName() const {
    return m_PlayerName;
}

const std::string& PlayerLeftEvent::getReason() const {
    return m_Reason;
}

std::string PlayerLeftEvent::toString() const {
    std::ostringstream oss;
    oss << "PlayerLeftEvent: " << m_PlayerName << " (" << m_PlayerId << ") - " << m_Reason;
    return oss.str();
}

ConnectionEstablishedEvent::ConnectionEstablishedEvent(std::string address, std::uint16_t port, EntityID playerId)
    : m_Address(std::move(address)),
      m_Port(port),
      m_PlayerId(playerId) {}

const std::string& ConnectionEstablishedEvent::getAddress() const {
    return m_Address;
}

std::uint16_t ConnectionEstablishedEvent::getPort() const {
    return m_Port;
}

EntityID ConnectionEstablishedEvent::getPlayerId() const {
    return m_PlayerId;
}

std::string ConnectionEstablishedEvent::toString() const {
    std::ostringstream oss;
    oss << "ConnectionEstablishedEvent: " << m_Address << ":" << m_Port << " (player " << m_PlayerId << ")";
    return oss.str();
}

ConnectionLostEvent::ConnectionLostEvent(std::string reason, bool timeout)
    : m_Reason(std::move(reason)),
      m_WasTimeout(timeout) {}

const std::string& ConnectionLostEvent::getReason() const {
    return m_Reason;
}

bool ConnectionLostEvent::wasTimeout() const {
    return m_WasTimeout;
}

std::string ConnectionLostEvent::toString() const {
    std::ostringstream oss;
    oss << "ConnectionLostEvent: " << m_Reason;
    if (m_WasTimeout) {
        oss << " (timeout)";
    }
    return oss.str();
}

ChunkReceivedEvent::ChunkReceivedEvent(const ChunkCoord& coord, std::uint32_t blockCount)
    : m_Coord(coord),
      m_BlockCount(blockCount) {}

const ChunkCoord& ChunkReceivedEvent::getCoord() const {
    return m_Coord;
}

std::uint32_t ChunkReceivedEvent::getBlockCount() const {
    return m_BlockCount;
}

std::string ChunkReceivedEvent::toString() const {
    std::ostringstream oss;
    oss << "ChunkReceivedEvent: (" << m_Coord.x << ", " << m_Coord.z << ") blocks=" << m_BlockCount;
    return oss.str();
}

ServerStartedEvent::ServerStartedEvent(std::uint16_t port, std::size_t maxPlayers)
    : m_Port(port),
      m_MaxPlayers(maxPlayers) {}

std::uint16_t ServerStartedEvent::getPort() const {
    return m_Port;
}

std::size_t ServerStartedEvent::getMaxPlayers() const {
    return m_MaxPlayers;
}

std::string ServerStartedEvent::toString() const {
    std::ostringstream oss;
    oss << "ServerStartedEvent: port=" << m_Port << " maxPlayers=" << m_MaxPlayers;
    return oss.str();
}

ServerStoppedEvent::ServerStoppedEvent(std::string reason)
    : m_Reason(std::move(reason)) {}

const std::string& ServerStoppedEvent::getReason() const {
    return m_Reason;
}

std::string ServerStoppedEvent::toString() const {
    std::ostringstream oss;
    oss << "ServerStoppedEvent: " << m_Reason;
    return oss.str();
}

} // namespace PoorCraft

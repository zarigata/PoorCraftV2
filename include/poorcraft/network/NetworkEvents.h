#pragma once

#include <cstdint>
#include <string>

#include <glm/vec3.hpp>

#include "poorcraft/core/Event.h"
#include "poorcraft/entity/Entity.h"
#include "poorcraft/world/ChunkCoord.h"

namespace PoorCraft {

class PlayerJoinedEvent : public Event {
public:
    PlayerJoinedEvent(EntityID playerId, std::string name, const glm::vec3& position);

    EVENT_CLASS_TYPE(PlayerJoined)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    EntityID getPlayerId() const;
    const std::string& getPlayerName() const;
    const glm::vec3& getPosition() const;

    std::string toString() const override;

private:
    EntityID m_PlayerId;
    std::string m_PlayerName;
    glm::vec3 m_Position;
};

class PlayerLeftEvent : public Event {
public:
    PlayerLeftEvent(EntityID playerId, std::string name, std::string reason);

    EVENT_CLASS_TYPE(PlayerLeft)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    EntityID getPlayerId() const;
    const std::string& getPlayerName() const;
    const std::string& getReason() const;

    std::string toString() const override;

private:
    EntityID m_PlayerId;
    std::string m_PlayerName;
    std::string m_Reason;
};

class ConnectionEstablishedEvent : public Event {
public:
    ConnectionEstablishedEvent(std::string address, std::uint16_t port, EntityID playerId);

    EVENT_CLASS_TYPE(ConnectionEstablished)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    const std::string& getAddress() const;
    std::uint16_t getPort() const;
    EntityID getPlayerId() const;

    std::string toString() const override;

private:
    std::string m_Address;
    std::uint16_t m_Port;
    EntityID m_PlayerId;
};

class ConnectionLostEvent : public Event {
public:
    ConnectionLostEvent(std::string reason, bool timeout);

    EVENT_CLASS_TYPE(ConnectionLost)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    const std::string& getReason() const;
    bool wasTimeout() const;

    std::string toString() const override;

private:
    std::string m_Reason;
    bool m_WasTimeout;
};

class ChunkReceivedEvent : public Event {
public:
    ChunkReceivedEvent(const ChunkCoord& coord, std::uint32_t blockCount);

    EVENT_CLASS_TYPE(ChunkReceived)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    const ChunkCoord& getCoord() const;
    std::uint32_t getBlockCount() const;

    std::string toString() const override;

private:
    ChunkCoord m_Coord;
    std::uint32_t m_BlockCount;
};

class ServerStartedEvent : public Event {
public:
    ServerStartedEvent(std::uint16_t port, std::size_t maxPlayers);

    EVENT_CLASS_TYPE(ServerStarted)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    std::uint16_t getPort() const;
    std::size_t getMaxPlayers() const;

    std::string toString() const override;

private:
    std::uint16_t m_Port;
    std::size_t m_MaxPlayers;
};

class ServerStoppedEvent : public Event {
public:
    explicit ServerStoppedEvent(std::string reason);

    EVENT_CLASS_TYPE(ServerStopped)
    EVENT_CLASS_CATEGORY(EventCategoryNetwork)

    const std::string& getReason() const;

    std::string toString() const override;

private:
    std::string m_Reason;
};

} // namespace PoorCraft

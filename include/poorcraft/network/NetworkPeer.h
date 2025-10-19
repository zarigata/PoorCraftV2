#pragma once

#include <cstdint>
#include <string>

#include "poorcraft/network/PacketType.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

typedef struct _ENetPacket ENetPacket;

typedef unsigned int enet_uint32;

typedef int enet_int32;

typedef unsigned char enet_uint8;

typedef unsigned short enet_uint16;

typedef unsigned long long enet_uint64;

namespace PoorCraft {

class PacketWriter;

class NetworkPeer {
public:
    explicit NetworkPeer(ENetPeer* peer = nullptr);

    bool send(const std::uint8_t* data, std::size_t size, enet_uint8 channel, bool reliable);
    bool sendPacket(PacketType type, const PacketWriter& writer, enet_uint8 channel) const;

    void disconnect(enet_uint32 data = 0);
    void disconnectNow(enet_uint32 data = 0);
    void reset();

    std::uint32_t getPing() const;
    std::string getAddress() const;
    bool isConnected() const;

    void setUserData(void* data);
    void* getUserData() const;

    ENetPeer* getHandle() const { return m_Peer; }

private:
    ENetPeer* m_Peer;
    void* m_UserData;
};

} // namespace PoorCraft

#include "poorcraft/network/NetworkPeer.h"

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/network/PacketSerializer.h"

namespace PoorCraft {

NetworkPeer::NetworkPeer(ENetPeer* peer)
    : m_Peer(peer), m_UserData(nullptr) {}

bool NetworkPeer::send(const std::uint8_t* data, std::size_t size, enet_uint8 channel, bool reliable) {
    if (!m_Peer || !data || size == 0) {
        PC_ERROR("NetworkPeer::send invalid parameters");
        return false;
    }

    ENetPacket* packet = enet_packet_create(data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (!packet) {
        PC_ERROR("Failed to create ENet packet");
        return false;
    }

    const int result = enet_peer_send(m_Peer, channel, packet);
    if (result < 0) {
        PC_ERROR("Failed to send packet to peer");
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}

bool NetworkPeer::sendPacket(PacketType type, const PacketWriter& writer, enet_uint8 channel) const {
    if (!m_Peer) {
        return false;
    }

    PacketWriter headerWriter;
    headerWriter.writeUInt8(static_cast<std::uint8_t>(type));
    headerWriter.writeUInt16(static_cast<std::uint16_t>(writer.getSize()));
    headerWriter.writeUInt32(0);
    headerWriter.writeUInt32(0);

    const std::size_t totalSize = headerWriter.getSize() + writer.getSize();
    std::vector<std::uint8_t> buffer;
    buffer.reserve(totalSize);
    buffer.insert(buffer.end(), headerWriter.getData(), headerWriter.getData() + headerWriter.getSize());
    buffer.insert(buffer.end(), writer.getData(), writer.getData() + writer.getSize());

    const bool reliable = isReliablePacket(type);
    return const_cast<NetworkPeer*>(this)->send(buffer.data(), buffer.size(), channel, reliable);
}

void NetworkPeer::disconnect(enet_uint32 data) {
    if (!m_Peer) {
        return;
    }
    enet_peer_disconnect(m_Peer, data);
}

void NetworkPeer::disconnectNow(enet_uint32 data) {
    if (!m_Peer) {
        return;
    }
    enet_peer_disconnect_now(m_Peer, data);
}

void NetworkPeer::reset() {
    if (!m_Peer) {
        return;
    }
    enet_peer_reset(m_Peer);
}

std::uint32_t NetworkPeer::getPing() const {
    if (!m_Peer) {
        return 0;
    }
    return m_Peer->roundTripTime;
}

std::string NetworkPeer::getAddress() const {
    if (!m_Peer) {
        return {};
    }

    char ip[64] = {0};
    enet_address_get_host_ip(&m_Peer->address, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(m_Peer->address.port);
}

bool NetworkPeer::isConnected() const {
    return m_Peer && m_Peer->state == ENET_PEER_STATE_CONNECTED;
}

void NetworkPeer::setUserData(void* data) {
    m_UserData = data;
    if (m_Peer) {
        m_Peer->data = data;
    }
}

void* NetworkPeer::getUserData() const {
    return m_UserData;
}

} // namespace PoorCraft

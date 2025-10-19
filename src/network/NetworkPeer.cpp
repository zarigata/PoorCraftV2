#include "poorcraft/network/NetworkPeer.h"

#include <chrono>
#include <limits>

#include <enet/enet.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/network/PacketSerializer.h"

namespace PoorCraft {

NetworkPeer::NetworkPeer(ENetPeer* peer)
    : m_Peer(peer),
      m_UserData(peer ? peer->data : nullptr),
      m_NextSequence(1) {}

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

    const std::size_t payloadSize = writer.getSize();
    if (payloadSize > std::numeric_limits<std::uint16_t>::max()) {
        PC_ERROR("Packet payload exceeds uint16_t size limit for type: " + getPacketTypeName(type));
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    PacketWriter headerWriter;
    headerWriter.writeUInt8(static_cast<std::uint8_t>(type));
    headerWriter.writeUInt16(static_cast<std::uint16_t>(payloadSize));
    headerWriter.writeUInt32(m_NextSequence++);
    headerWriter.writeUInt32(static_cast<std::uint32_t>(timestampMs & 0xffffffffULL));

    const std::size_t totalSize = headerWriter.getSize() + payloadSize;

    const auto& config = poorcraft::Config::get_instance();
    const int configuredMax = config.get_int(poorcraft::Config::NetworkConfig::MAX_PACKET_SIZE_KEY, 1200);
    if (configuredMax > 0 && totalSize > static_cast<std::size_t>(configuredMax)) {
        PC_ERROR("Packet exceeds configured max size for type: " + getPacketTypeName(type));
        return false;
    }

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

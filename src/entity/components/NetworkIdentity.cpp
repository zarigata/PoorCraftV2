#include "poorcraft/entity/components/NetworkIdentity.h"

#include <algorithm>

namespace PoorCraft {

NetworkIdentity::NetworkIdentity()
    : m_NetworkId(0),
      m_OwnerId(0),
      m_IsLocalPlayer(false),
      m_IsServerControlled(false),
      m_LastUpdateTick(0) {}

void NetworkIdentity::setNetworkId(EntityID id) {
    m_NetworkId = id;
}

EntityID NetworkIdentity::getNetworkId() const {
    return m_NetworkId;
}

void NetworkIdentity::setOwnerId(EntityID id) {
    m_OwnerId = id;
}

EntityID NetworkIdentity::getOwnerId() const {
    return m_OwnerId;
}

void NetworkIdentity::setLocalPlayer(bool value) {
    m_IsLocalPlayer = value;
}

bool NetworkIdentity::isLocalPlayer() const {
    return m_IsLocalPlayer;
}

void NetworkIdentity::setServerControlled(bool value) {
    m_IsServerControlled = value;
}

bool NetworkIdentity::isServerControlled() const {
    return m_IsServerControlled;
}

void NetworkIdentity::setLastUpdateTick(std::uint32_t tick) {
    m_LastUpdateTick = tick;
}

std::uint32_t NetworkIdentity::getLastUpdateTick() const {
    return m_LastUpdateTick;
}

void NetworkIdentity::addSnapshot(const NetworkSnapshot& snapshot) {
    m_Snapshots.push_back(snapshot);
    if (m_Snapshots.size() > 10) {
        m_Snapshots.pop_front();
    }
    m_LastUpdateTick = snapshot.tick;
}

bool NetworkIdentity::getInterpolatedState(double serverTime, NetworkSnapshot& outSnapshot) const {
    if (m_Snapshots.empty()) {
        return false;
    }

    if (m_Snapshots.size() == 1) {
        outSnapshot = m_Snapshots.front();
        return true;
    }

    const double targetTick = serverTime;

    for (std::size_t i = 1; i < m_Snapshots.size(); ++i) {
        const auto& prev = m_Snapshots[i - 1];
        const auto& next = m_Snapshots[i];

        if (prev.tick <= targetTick && next.tick >= targetTick) {
            const double range = static_cast<double>(next.tick - prev.tick);
            const double alpha = range > 0.0 ? (targetTick - prev.tick) / range : 0.0;

            outSnapshot.tick = static_cast<std::uint32_t>(targetTick);
            outSnapshot.position = glm::mix(prev.position, next.position, static_cast<float>(alpha));
            outSnapshot.velocity = glm::mix(prev.velocity, next.velocity, static_cast<float>(alpha));
            outSnapshot.rotation = glm::slerp(prev.rotation, next.rotation, static_cast<float>(alpha));
            outSnapshot.animationState = alpha < 0.5 ? prev.animationState : next.animationState;
            return true;
        }
    }

    outSnapshot = m_Snapshots.back();
    return true;
}

} // namespace PoorCraft

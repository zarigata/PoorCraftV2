#pragma once

#include <cstdint>
#include <deque>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "poorcraft/entity/Component.h"

namespace PoorCraft {

struct NetworkSnapshot {
    std::uint32_t tick;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::quat rotation;
    std::uint8_t animationState;
};

class NetworkIdentity : public Component {
public:
    NetworkIdentity();

    void setNetworkId(EntityID id);
    EntityID getNetworkId() const;

    void setOwnerId(EntityID id);
    EntityID getOwnerId() const;

    void setLocalPlayer(bool value);
    bool isLocalPlayer() const;

    void setServerControlled(bool value);
    bool isServerControlled() const;

    void setLastUpdateTick(std::uint32_t tick);
    std::uint32_t getLastUpdateTick() const;

    void addSnapshot(const NetworkSnapshot& snapshot);
    bool getInterpolatedState(double serverTime, NetworkSnapshot& outSnapshot) const;

private:
    EntityID m_NetworkId;
    EntityID m_OwnerId;
    bool m_IsLocalPlayer;
    bool m_IsServerControlled;
    std::uint32_t m_LastUpdateTick;

    std::deque<NetworkSnapshot> m_Snapshots;
};

} // namespace PoorCraft

#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "poorcraft/entity/Component.h"

namespace PoorCraft {

class PhysicsWorld;
class Player;
class Camera;
class Transform;

class PlayerController : public Component {
public:
    PlayerController(std::shared_ptr<PhysicsWorld> physicsWorld,
                     std::shared_ptr<Camera> camera);

    Player& getPlayer();

    void update(float deltaTime);
    void handleInput(const glm::vec3& wishDirection,
                     bool sprinting,
                     bool jumpRequested,
                     bool flyToggle,
                     bool swimToggle);

    void syncTransform(Transform& transform) const;

private:
    std::unique_ptr<Player> m_Player;
};

} // namespace PoorCraft

#include "poorcraft/entity/components/PlayerController.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/physics/Player.h"
#include "poorcraft/physics/PhysicsWorld.h"
#include "poorcraft/rendering/Camera.h"

namespace PoorCraft {

PlayerController::PlayerController(std::shared_ptr<PhysicsWorld> physicsWorld,
                                   std::shared_ptr<Camera> camera)
    : m_Player(std::make_unique<Player>(std::move(physicsWorld), std::move(camera))) {
    PC_INFO("PlayerController initialized");
}

Player& PlayerController::getPlayer() {
    return *m_Player;
}

void PlayerController::update(float deltaTime) {
    if (!m_Player) {
        return;
    }
    m_Player->update(deltaTime);
}

void PlayerController::handleInput(const glm::vec3& wishDirection,
                                   bool sprinting,
                                   bool jumpRequested,
                                   bool flyToggle,
                                   bool swimToggle) {
    if (!m_Player) {
        return;
    }
    m_Player->handleInput(wishDirection, sprinting, jumpRequested, flyToggle, swimToggle);
}

void PlayerController::syncTransform(Transform& transform) const {
    if (!m_Player) {
        return;
    }
    transform.setPosition(m_Player->getPosition());
}

} // namespace PoorCraft

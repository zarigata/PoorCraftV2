#include "poorcraft/entity/systems/AnimationSystem.h"

#include <glm/gtx/norm.hpp>

#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/components/AnimationController.h"
#include "poorcraft/entity/components/PlayerController.h"

namespace PoorCraft {

namespace {
constexpr float WALK_SPEED_THRESHOLD = 0.1f;
constexpr float RUN_SPEED_THRESHOLD = 4.0f;
}

AnimationSystem::AnimationSystem(EntityManager& manager)
    : m_Manager(manager) {}

void AnimationSystem::update(float deltaTime) {
    auto entities = m_Manager.getEntitiesWith<AnimationController>();
    if (entities.empty()) {
        return;
    }

    for (auto* entity : entities) {
        if (!entity) {
            continue;
        }
        updateEntity(*entity, deltaTime);
    }
}

void AnimationSystem::updateEntity(Entity& entity, float deltaTime) {
    auto* animation = entity.getComponent<AnimationController>();
    if (!animation) {
        return;
    }

    AnimationController::AnimationState newState = animation->getState();

    auto* playerController = entity.getComponent<PlayerController>();
    if (playerController) {
        Player& player = playerController->getPlayer();

        if (player.isFlying()) {
            newState = AnimationController::AnimationState::Flying;
        } else if (player.isSwimming()) {
            newState = AnimationController::AnimationState::Swimming;
        } else if (!player.isGrounded()) {
            const float verticalVelocity = player.getVelocity().y;
            newState = verticalVelocity > 0.0f
                ? AnimationController::AnimationState::Jumping
                : AnimationController::AnimationState::Falling;
        } else {
            const float speedSq = glm::length2(player.getVelocity());
            if (speedSq > RUN_SPEED_THRESHOLD * RUN_SPEED_THRESHOLD) {
                newState = AnimationController::AnimationState::Running;
            } else if (speedSq > WALK_SPEED_THRESHOLD * WALK_SPEED_THRESHOLD) {
                newState = AnimationController::AnimationState::Walking;
            } else {
                newState = AnimationController::AnimationState::Idle;
            }
        }
    }

    animation->setState(newState);
    animation->update(deltaTime);
}

} // namespace PoorCraft

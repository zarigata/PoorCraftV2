#include "poorcraft/physics/MovementController.h"

#include <algorithm>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>

namespace PoorCraft {

void MovementController::setParameters(const MovementParameters& params) {
    m_Params = params;
}

void MovementController::setMode(MovementMode mode) {
    m_Mode = mode;
}

MovementMode MovementController::getMode() const {
    return m_Mode;
}

const MovementParameters& MovementController::getParameters() const {
    return m_Params;
}

void MovementController::setVelocity(const glm::vec3& velocity) {
    m_Velocity = velocity;
}

const glm::vec3& MovementController::getVelocity() const {
    return m_Velocity;
}

void MovementController::setWishDirection(const glm::vec3& wishDir, bool sprinting) {
    m_WishDirection = wishDir;
    m_WishSprint = sprinting;
}

void MovementController::update(float deltaTime, bool grounded, bool inWater) {
    if (inWater) {
        applyWaterMovement(deltaTime);
    } else if (grounded && m_Mode != MovementMode::FLYING) {
        applyGroundMovement(deltaTime);
    } else {
        applyAirMovement(deltaTime);
    }

    if (m_Mode != MovementMode::FLYING && !inWater) {
        m_Velocity.y -= m_Params.gravity * deltaTime;
    }

    if (m_Mode == MovementMode::FLYING) {
        m_Velocity.y = glm::clamp(m_Velocity.y, -m_Params.flySpeed, m_Params.flySpeed);
    }
}

void MovementController::applyJump() {
    m_WishJump = true;
    if (m_Mode == MovementMode::FLYING) {
        m_Velocity.y = m_Params.flySpeed;
    }
}

void MovementController::applyGroundMovement(float deltaTime) {
    applyFriction(deltaTime, m_Params.groundFriction);

    glm::vec3 wishDir = m_WishDirection;
    float wishSpeed = m_WishSprint ? m_Params.sprintSpeed : m_Params.walkSpeed;

    if (glm::length2(wishDir) > 0.0f) {
        wishDir = glm::normalize(wishDir);
        glm::vec3 acceleration = wishDir * m_Params.acceleration * deltaTime;
        m_Velocity += acceleration;

        float currentSpeed = glm::dot(m_Velocity, wishDir);
        if (currentSpeed > wishSpeed) {
            m_Velocity -= wishDir * (currentSpeed - wishSpeed);
        }
    }

    if (m_WishJump) {
        m_Velocity.y = m_Params.jumpForce;
        m_WishJump = false;
    }
}

void MovementController::applyAirMovement(float deltaTime) {
    applyFriction(deltaTime, m_Params.airFriction);

    glm::vec3 wishDir = m_WishDirection;
    float wishSpeed = (m_Mode == MovementMode::FLYING) ? m_Params.flySpeed
                                                        : (m_WishSprint ? m_Params.sprintSpeed : m_Params.walkSpeed);

    if (glm::length2(wishDir) > 0.0f) {
        wishDir = glm::normalize(wishDir);
        glm::vec3 acceleration = wishDir * m_Params.acceleration * 0.5f * deltaTime;
        m_Velocity += acceleration;

        float currentSpeed = glm::dot(m_Velocity, wishDir);
        if (currentSpeed > wishSpeed) {
            m_Velocity -= wishDir * (currentSpeed - wishSpeed);
        }
    }

    if (m_Mode == MovementMode::FLYING && m_WishJump) {
        m_Velocity.y = m_Params.flySpeed;
        m_WishJump = false;
    }
}

void MovementController::applyWaterMovement(float deltaTime) {
    applyFriction(deltaTime, m_Params.waterFriction);

    glm::vec3 wishDir = m_WishDirection;
    float wishSpeed = m_Params.swimSpeed;

    if (glm::length2(wishDir) > 0.0f) {
        wishDir = glm::normalize(wishDir);
        glm::vec3 acceleration = wishDir * (m_Params.acceleration * 0.7f) * deltaTime;
        m_Velocity += acceleration;

        float currentSpeed = glm::dot(m_Velocity, wishDir);
        if (currentSpeed > wishSpeed) {
            m_Velocity -= wishDir * (currentSpeed - wishSpeed);
        }
    }

    if (m_WishJump) {
        m_Velocity.y = m_Params.jumpForce * 0.5f;
        m_WishJump = false;
    }
}

void MovementController::applyFriction(float deltaTime, float frictionCoefficient) {
    if (glm::length2(m_WishDirection) > 0.0f) {
        return;
    }

    float speed = glm::length(m_Velocity);
    if (speed <= 0.0f) {
        m_Velocity = glm::vec3(0.0f);
        return;
    }

    float drop = speed * frictionCoefficient * deltaTime;
    float newSpeed = std::max(speed - drop, 0.0f);
    if (newSpeed != speed) {
        m_Velocity *= newSpeed / speed;
    }
}

} // namespace PoorCraft

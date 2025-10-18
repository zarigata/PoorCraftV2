#pragma once

#include <glm/glm.hpp>

namespace PoorCraft {

class PhysicsWorld;

enum class MovementMode {
    WALKING,
    FLYING,
    SWIMMING
};

struct MovementParameters {
    float walkSpeed = 4.317f;
    float sprintSpeed = 5.612f;
    float flySpeed = 10.0f;
    float swimSpeed = 3.0f;
    float gravity = 32.0f;
    float jumpForce = 10.0f;
    float groundFriction = 8.0f;
    float airFriction = 1.0f;
    float waterFriction = 2.0f;
    float acceleration = 20.0f;
};

class MovementController {
public:
    MovementController() = default;

    void setParameters(const MovementParameters& params);
    void setMode(MovementMode mode);

    [[nodiscard]] MovementMode getMode() const;
    [[nodiscard]] const MovementParameters& getParameters() const;

    void setVelocity(const glm::vec3& velocity);
    [[nodiscard]] const glm::vec3& getVelocity() const;

    void setWishDirection(const glm::vec3& wishDir, bool sprinting);

    void update(float deltaTime, bool grounded, bool inWater);

    void applyJump();

private:
    MovementMode m_Mode = MovementMode::WALKING;
    MovementParameters m_Params;

    glm::vec3 m_Velocity{0.0f};
    glm::vec3 m_WishDirection{0.0f};
    bool m_WishSprint = false;
    bool m_WishJump = false;

    void applyGroundMovement(float deltaTime);
    void applyAirMovement(float deltaTime);
    void applyWaterMovement(float deltaTime);
    void applyFriction(float deltaTime, float frictionCoefficient);
};

} // namespace PoorCraft

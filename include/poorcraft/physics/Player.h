#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "poorcraft/physics/AABB.h"
#include "poorcraft/physics/MovementController.h"
#include "poorcraft/physics/RaycastHit.h"

namespace PoorCraft {

class PhysicsWorld;
class Camera;

struct PlayerSettings {
    float width = 0.6f;
    float height = 1.8f;
    float eyeHeight = 1.62f;
    float stepHeight = 0.5f;
    float reachDistance = 5.0f;
};

class Player {
public:
    Player(std::shared_ptr<PhysicsWorld> physicsWorld,
           std::shared_ptr<Camera> camera);

    void setMovementParameters(const MovementParameters& params);
    void setSettings(const PlayerSettings& settings);
    void setPosition(const glm::vec3& position);

    void handleInput(const glm::vec3& wishDirection,
                     bool sprinting,
                     bool jumpRequested,
                     bool flyToggle,
                     bool swimToggle);

    void update(float deltaTime);

    [[nodiscard]] const glm::vec3& getPosition() const;
    [[nodiscard]] glm::vec3 getVelocity() const;
    [[nodiscard]] const PhysicsAABB& getBounds() const;
    [[nodiscard]] const MovementController& getMovementController() const;
    [[nodiscard]] bool isGrounded() const;
    [[nodiscard]] bool isSwimming() const;
    [[nodiscard]] bool isFlying() const;

    [[nodiscard]] RaycastHit getTargetBlock() const;

private:
    void updateBounds();
    void updateCamera() const;
    void resolveMovement(float deltaTime);
    void updateGroundAndWaterState();
    void updateTargetBlock();
    [[nodiscard]] glm::vec3 getHalfExtents() const;
    void applyEnvironmentMode();

    std::shared_ptr<PhysicsWorld> m_PhysicsWorld;
    std::shared_ptr<Camera> m_Camera;

    MovementController m_MovementController;
    MovementParameters m_MovementParams;
    PlayerSettings m_Settings;

    PhysicsAABB m_Bounds;
    glm::vec3 m_Position{0.0f};
    glm::vec3 m_Velocity{0.0f};

    bool m_Grounded = false;
    bool m_InWater = false;
    bool m_FlyToggleRequested = false;
    bool m_SwimToggleRequested = false;

    RaycastHit m_TargetBlock;
};

} // namespace PoorCraft

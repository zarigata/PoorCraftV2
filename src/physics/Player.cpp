#include "poorcraft/physics/Player.h"

#include <glm/gtx/norm.hpp>

#include "poorcraft/physics/CollisionDetector.h"
#include "poorcraft/physics/PhysicsWorld.h"
#include "poorcraft/physics/Raycaster.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/core/Config.h"

namespace PoorCraft {

namespace {
constexpr glm::vec3 UP_AXIS(0.0f, 1.0f, 0.0f);
constexpr float WATER_SAMPLE_OFFSET = 0.1f;
}

Player::Player(std::shared_ptr<PhysicsWorld> physicsWorld,
               std::shared_ptr<Camera> camera)
    : m_PhysicsWorld(std::move(physicsWorld)),
      m_Camera(std::move(camera)) {
    auto& config = poorcraft::Config::get_instance();

    MovementParameters params = m_MovementParams;
    params.walkSpeed = config.get_float(poorcraft::Config::PhysicsConfig::WALK_SPEED_KEY, params.walkSpeed);
    params.sprintSpeed = config.get_float(poorcraft::Config::PhysicsConfig::SPRINT_SPEED_KEY, params.sprintSpeed);
    params.flySpeed = config.get_float(poorcraft::Config::PhysicsConfig::FLY_SPEED_KEY, params.flySpeed);
    params.swimSpeed = config.get_float(poorcraft::Config::PhysicsConfig::SWIM_SPEED_KEY, params.swimSpeed);
    params.gravity = config.get_float(poorcraft::Config::PhysicsConfig::GRAVITY_KEY, params.gravity);
    params.jumpForce = config.get_float(poorcraft::Config::PhysicsConfig::JUMP_FORCE_KEY, params.jumpForce);
    params.groundFriction = config.get_float(poorcraft::Config::PhysicsConfig::GROUND_FRICTION_KEY, params.groundFriction);
    params.airFriction = config.get_float(poorcraft::Config::PhysicsConfig::AIR_FRICTION_KEY, params.airFriction);
    params.waterFriction = config.get_float(poorcraft::Config::PhysicsConfig::WATER_FRICTION_KEY, params.waterFriction);
    params.acceleration = config.get_float(poorcraft::Config::PhysicsConfig::ACCELERATION_KEY, params.acceleration);
    setMovementParameters(params);

    PlayerSettings settings = m_Settings;
    settings.width = config.get_float(poorcraft::Config::PhysicsConfig::PLAYER_WIDTH_KEY, settings.width);
    settings.height = config.get_float(poorcraft::Config::PhysicsConfig::PLAYER_HEIGHT_KEY, settings.height);
    settings.eyeHeight = config.get_float(poorcraft::Config::PhysicsConfig::PLAYER_EYE_HEIGHT_KEY, settings.eyeHeight);
    settings.stepHeight = config.get_float(poorcraft::Config::PhysicsConfig::STEP_HEIGHT_KEY, settings.stepHeight);
    settings.reachDistance = config.get_float(poorcraft::Config::PhysicsConfig::REACH_DISTANCE_KEY, settings.reachDistance);
    setSettings(settings);
}

void Player::setMovementParameters(const MovementParameters& params) {
    m_MovementParams = params;
    m_MovementController.setParameters(m_MovementParams);
}

void Player::setSettings(const PlayerSettings& settings) {
    m_Settings = settings;
    updateBounds();
    updateCamera();
}

void Player::setPosition(const glm::vec3& position) {
    m_Position = position;
    updateBounds();
    updateCamera();
}

void Player::handleInput(const glm::vec3& wishDirection,
                         bool sprinting,
                         bool jumpRequested,
                         bool flyToggle,
                         bool swimToggle) {
    glm::vec3 forward(0.0f, 0.0f, -1.0f);
    glm::vec3 right(1.0f, 0.0f, 0.0f);

    if (m_Camera) {
        forward = m_Camera->getForward();
        right = m_Camera->getRight();
    }

    forward.y = 0.0f;
    right.y = 0.0f;

    if (glm::length2(forward) > 0.0f) {
        forward = glm::normalize(forward);
    } else {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    if (glm::length2(right) > 0.0f) {
        right = glm::normalize(right);
    } else {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 desired = forward * wishDirection.z + right * wishDirection.x;
    if (m_MovementController.getMode() == MovementMode::FLYING) {
        desired.y = wishDirection.y;
    }

    m_MovementController.setWishDirection(desired, sprinting);

    if (jumpRequested && m_Grounded) {
        m_MovementController.applyJump();
    }

    if (flyToggle) {
        m_FlyToggleRequested = true;
    }

    if (swimToggle) {
        m_SwimToggleRequested = true;
    }
}

void Player::update(float deltaTime) {
    applyEnvironmentMode();

    m_MovementController.update(deltaTime, m_Grounded, m_InWater);
    m_Velocity = m_MovementController.getVelocity();

    resolveMovement(deltaTime);
    updateGroundAndWaterState();
    updateCamera();
    updateTargetBlock();
}

const glm::vec3& Player::getPosition() const {
    return m_Position;
}

glm::vec3 Player::getVelocity() const {
    return m_Velocity;
}

const PhysicsAABB& Player::getBounds() const {
    return m_Bounds;
}

const MovementController& Player::getMovementController() const {
    return m_MovementController;
}

bool Player::isGrounded() const {
    return m_Grounded;
}

bool Player::isSwimming() const {
    return m_MovementController.getMode() == MovementMode::SWIMMING;
}

bool Player::isFlying() const {
    return m_MovementController.getMode() == MovementMode::FLYING;
}

RaycastHit Player::getTargetBlock() const {
    return m_TargetBlock;
}

void Player::updateBounds() {
    const glm::vec3 halfExtents = getHalfExtents();
    const glm::vec3 center = m_Position + glm::vec3(0.0f, halfExtents.y, 0.0f);
    m_Bounds = PhysicsAABB::fromCenterExtents(center, halfExtents);
}

void Player::updateCamera() const {
    if (!m_Camera) {
        return;
    }

    m_Camera->setPosition(m_Position + glm::vec3(0.0f, m_Settings.eyeHeight, 0.0f));
}

void Player::resolveMovement(float deltaTime) {
    if (deltaTime <= 0.0f || !m_PhysicsWorld) {
        updateBounds();
        return;
    }

    const glm::vec3 displacement = m_Velocity * deltaTime;

    if (m_MovementController.getMode() == MovementMode::FLYING) {
        m_Position += displacement;
        updateBounds();
        m_Grounded = false;
        return;
    }

    CollisionResult result = CollisionDetector::resolveCollision(m_Bounds, displacement, *m_PhysicsWorld);

    glm::vec3 halfExtents = getHalfExtents();
    glm::vec3 center = result.position;

    m_Bounds = PhysicsAABB::fromCenterExtents(center, halfExtents);
    m_Position = center - glm::vec3(0.0f, halfExtents.y, 0.0f);

    if (deltaTime > 0.0f) {
        m_Velocity = result.velocity / deltaTime;
        m_MovementController.setVelocity(m_Velocity);
    }
}

void Player::updateGroundAndWaterState() {
    if (!m_PhysicsWorld) {
        m_Grounded = false;
        m_InWater = false;
        return;
    }

    m_Grounded = CollisionDetector::checkGrounded(m_Bounds, *m_PhysicsWorld);

    const glm::vec3 halfExtents = getHalfExtents();
    const glm::vec3 center = m_Bounds.getCenter();
    const glm::vec3 basePosition = center - glm::vec3(0.0f, halfExtents.y, 0.0f);

    const float torsoHeight = m_Settings.height * 0.5f;

    uint16_t feetBlock = m_PhysicsWorld->getBlockAt(center.x,
                                                    basePosition.y + WATER_SAMPLE_OFFSET,
                                                    center.z);
    uint16_t torsoBlock = m_PhysicsWorld->getBlockAt(center.x,
                                                     basePosition.y + torsoHeight,
                                                     center.z);

    auto& registry = BlockRegistry::getInstance();
    const auto& feetType = registry.getBlock(feetBlock);
    const auto& torsoType = registry.getBlock(torsoBlock);

    const bool feetInWater = feetType.name == "water";
    const bool torsoInWater = torsoType.name == "water";

    m_InWater = feetInWater || torsoInWater;
}

void Player::updateTargetBlock() {
    if (!m_PhysicsWorld || !m_Camera) {
        m_TargetBlock = {};
        return;
    }

    m_TargetBlock = Raycaster::raycast(m_Camera->getPosition(),
                                       m_Camera->getForward(),
                                       m_Settings.reachDistance,
                                       *m_PhysicsWorld);
}

glm::vec3 Player::getHalfExtents() const {
    return glm::vec3(m_Settings.width * 0.5f,
                     m_Settings.height * 0.5f,
                     m_Settings.width * 0.5f);
}

void Player::applyEnvironmentMode() {
    if (m_FlyToggleRequested) {
        if (m_MovementController.getMode() == MovementMode::FLYING) {
            m_MovementController.setMode(MovementMode::WALKING);
            m_Velocity.y = 0.0f;
            m_MovementController.setVelocity(m_Velocity);
        } else {
            m_MovementController.setMode(MovementMode::FLYING);
        }
        m_FlyToggleRequested = false;
    }

    if (m_MovementController.getMode() != MovementMode::FLYING) {
        if (m_InWater || m_SwimToggleRequested) {
            if (m_InWater) {
                m_MovementController.setMode(MovementMode::SWIMMING);
            }
        } else if (m_MovementController.getMode() == MovementMode::SWIMMING && !m_InWater) {
            m_MovementController.setMode(MovementMode::WALKING);
        }
    }

    if (m_SwimToggleRequested && !m_InWater) {
        if (m_MovementController.getMode() == MovementMode::SWIMMING) {
            m_MovementController.setMode(MovementMode::WALKING);
        }
        m_SwimToggleRequested = false;
    } else if (m_SwimToggleRequested) {
        m_SwimToggleRequested = false;
    }
}

} // namespace PoorCraft

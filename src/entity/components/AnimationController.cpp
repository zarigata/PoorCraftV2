#include "poorcraft/entity/components/AnimationController.h"

#include <cmath>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
constexpr float WALK_FREQUENCY = 2.0f;
constexpr float RUN_FREQUENCY = 4.0f;
constexpr float WALK_AMP_DEGREES = 30.0f;
constexpr float RUN_AMP_DEGREES = 45.0f;

glm::quat rotationFromEuler(float xDegrees, float yDegrees, float zDegrees) {
    const glm::vec3 radians = glm::radians(glm::vec3(xDegrees, yDegrees, zDegrees));
    return glm::quat(radians);
}

} // namespace

AnimationController::AnimationController()
    : m_CurrentState(AnimationState::Idle),
      m_PreviousState(AnimationState::Idle),
      m_StateTime(0.0f),
      m_BlendTime(0.2f) {
    initializeBones();
}

void AnimationController::setState(AnimationState state) {
    if (state == m_CurrentState) {
        return;
    }

    m_PreviousState = m_CurrentState;
    m_CurrentState = state;
    m_StateTime = 0.0f;
    m_PreviousBoneTransforms = m_BoneTransforms;

    PC_DEBUG("Animation state changed to " + std::to_string(static_cast<int>(state)));
}

AnimationController::AnimationState AnimationController::getState() const {
    return m_CurrentState;
}

AnimationController::AnimationState AnimationController::getPreviousState() const {
    return m_PreviousState;
}

float AnimationController::getStateTime() const {
    return m_StateTime;
}

void AnimationController::update(float deltaTime) {
    m_StateTime += deltaTime;

    std::unordered_map<std::string, BoneTransform> target = m_BoneTransforms;
    computeIdleState(target);

    switch (m_CurrentState) {
        case AnimationState::Idle:
            computeIdleState(target);
            break;
        case AnimationState::Walking:
            computeWalkingState(deltaTime, target);
            break;
        case AnimationState::Running:
            computeRunningState(deltaTime, target);
            break;
        case AnimationState::Jumping:
            computeJumpingState(target);
            break;
        case AnimationState::Falling:
            computeFallingState(target);
            break;
        case AnimationState::Swimming:
            computeSwimmingState(deltaTime, target);
            break;
        case AnimationState::Flying:
            computeFlyingState(target);
            break;
    }

    blendStates(deltaTime, target);
}

const AnimationController::BoneTransform& AnimationController::getBoneTransform(const std::string& bone) const {
    auto it = m_BoneTransforms.find(bone);
    if (it != m_BoneTransforms.end()) {
        return it->second;
    }

    static BoneTransform identity;
    return identity;
}

void AnimationController::initializeBones() {
    m_BoneTransforms.clear();
    m_PreviousBoneTransforms.clear();

    const std::string bones[] = {
        "head",
        "body",
        "leftArm",
        "rightArm",
        "leftLeg",
        "rightLeg"
    };

    for (const auto& bone : bones) {
        m_BoneTransforms.emplace(bone, BoneTransform{});
        m_PreviousBoneTransforms.emplace(bone, BoneTransform{});
    }
}

void AnimationController::computeIdleState(std::unordered_map<std::string, BoneTransform>& target) const {
    for (auto& [name, transform] : target) {
        transform.position = glm::vec3(0.0f);
        transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
}

void AnimationController::computeWalkingState(float, std::unordered_map<std::string, BoneTransform>& target) const {
    float phase = m_StateTime * WALK_FREQUENCY * glm::two_pi<float>();
    float amplitude = WALK_AMP_DEGREES;

    target["leftArm"].rotation = rotationFromEuler(std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["rightArm"].rotation = rotationFromEuler(-std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["leftLeg"].rotation = rotationFromEuler(-std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(std::sin(phase) * amplitude, 0.0f, 0.0f);
}

void AnimationController::computeRunningState(float, std::unordered_map<std::string, BoneTransform>& target) const {
    float phase = m_StateTime * RUN_FREQUENCY * glm::two_pi<float>();
    float amplitude = RUN_AMP_DEGREES;

    target["leftArm"].rotation = rotationFromEuler(std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["rightArm"].rotation = rotationFromEuler(-std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["leftLeg"].rotation = rotationFromEuler(-std::sin(phase) * amplitude, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(std::sin(phase) * amplitude, 0.0f, 0.0f);
}

void AnimationController::computeJumpingState(std::unordered_map<std::string, BoneTransform>& target) const {
    target["leftArm"].rotation = rotationFromEuler(-45.0f, 0.0f, 0.0f);
    target["rightArm"].rotation = rotationFromEuler(-45.0f, 0.0f, 0.0f);
    target["leftLeg"].rotation = rotationFromEuler(30.0f, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(30.0f, 0.0f, 0.0f);
}

void AnimationController::computeFallingState(std::unordered_map<std::string, BoneTransform>& target) const {
    target["leftArm"].rotation = rotationFromEuler(10.0f, 0.0f, 0.0f);
    target["rightArm"].rotation = rotationFromEuler(10.0f, 0.0f, 0.0f);
    target["leftLeg"].rotation = rotationFromEuler(-20.0f, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(-20.0f, 0.0f, 0.0f);
}

void AnimationController::computeSwimmingState(float, std::unordered_map<std::string, BoneTransform>& target) const {
    float phase = m_StateTime * WALK_FREQUENCY * glm::two_pi<float>();
    float amplitude = 45.0f;

    target["leftArm"].rotation = rotationFromEuler(0.0f, 0.0f, std::sin(phase) * amplitude);
    target["rightArm"].rotation = rotationFromEuler(0.0f, 0.0f, -std::sin(phase) * amplitude);
    target["leftLeg"].rotation = rotationFromEuler(std::sin(phase) * amplitude * 0.5f, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(-std::sin(phase) * amplitude * 0.5f, 0.0f, 0.0f);
}

void AnimationController::computeFlyingState(std::unordered_map<std::string, BoneTransform>& target) const {
    target["leftArm"].rotation = rotationFromEuler(0.0f, 0.0f, -45.0f);
    target["rightArm"].rotation = rotationFromEuler(0.0f, 0.0f, 45.0f);
    target["leftLeg"].rotation = rotationFromEuler(0.0f, 0.0f, 0.0f);
    target["rightLeg"].rotation = rotationFromEuler(0.0f, 0.0f, 0.0f);
}

void AnimationController::blendStates(float deltaTime, const std::unordered_map<std::string, BoneTransform>& target) {
    if (m_PreviousState == m_CurrentState || m_BlendTime <= 0.0f) {
        m_BoneTransforms = target;
        return;
    }

    float alpha = std::min(m_StateTime / m_BlendTime, 1.0f);
    for (const auto& [bone, targetTransform] : target) {
        auto& currentTransform = m_BoneTransforms[bone];
        const auto& previousTransform = m_PreviousBoneTransforms[bone];

        currentTransform.position = glm::mix(previousTransform.position, targetTransform.position, alpha);
        currentTransform.rotation = glm::slerp(previousTransform.rotation, targetTransform.rotation, alpha);
    }

    if (alpha >= 1.0f) {
        m_PreviousState = m_CurrentState;
    }
}

} // namespace PoorCraft

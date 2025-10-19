#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "poorcraft/entity/Component.h"

namespace PoorCraft {

class AnimationController : public Component {
public:
    enum class AnimationState {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling,
        Swimming,
        Flying
    };

    struct BoneTransform {
        glm::vec3 position{0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    };

    AnimationController();

    void setState(AnimationState state);
    AnimationState getState() const;
    AnimationState getPreviousState() const;

    float getStateTime() const;
    void update(float deltaTime);

    const BoneTransform& getBoneTransform(const std::string& bone) const;

private:
    void initializeBones();
    void computeIdleState(std::unordered_map<std::string, BoneTransform>& target) const;
    void computeWalkingState(float deltaTime, std::unordered_map<std::string, BoneTransform>& target) const;
    void computeRunningState(float deltaTime, std::unordered_map<std::string, BoneTransform>& target) const;
    void computeJumpingState(std::unordered_map<std::string, BoneTransform>& target) const;
    void computeFallingState(std::unordered_map<std::string, BoneTransform>& target) const;
    void computeSwimmingState(float deltaTime, std::unordered_map<std::string, BoneTransform>& target) const;
    void computeFlyingState(std::unordered_map<std::string, BoneTransform>& target) const;
    void blendStates(float deltaTime, const std::unordered_map<std::string, BoneTransform>& target);

    AnimationState m_CurrentState;
    AnimationState m_PreviousState;
    float m_StateTime;
    float m_BlendTime;
    std::unordered_map<std::string, BoneTransform> m_BoneTransforms;
    std::unordered_map<std::string, BoneTransform> m_PreviousBoneTransforms;
};

} // namespace PoorCraft

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "poorcraft/entity/Component.h"

namespace PoorCraft {

class Transform : public Component {
public:
    Transform();

    const glm::vec3& getPosition() const { return m_Position; }
    const glm::quat& getRotation() const { return m_Rotation; }
    const glm::vec3& getScale() const { return m_Scale; }

    glm::mat4 getModelMatrix() const;
    glm::vec3 getInterpolatedPosition(float alpha) const;

    void updatePrevious();

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setScale(const glm::vec3& scale);

    void translate(const glm::vec3& offset);
    void rotate(const glm::quat& deltaRotation);

private:
    glm::vec3 m_Position;
    glm::quat m_Rotation;
    glm::vec3 m_Scale;
    glm::vec3 m_PreviousPosition;
    bool m_HasPrevious;
};

} // namespace PoorCraft

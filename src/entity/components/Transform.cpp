#include "poorcraft/entity/components/Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace PoorCraft {

Transform::Transform()
    : m_Position(0.0f),
      m_Rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
      m_Scale(1.0f),
      m_PreviousPosition(0.0f),
      m_HasPrevious(false) {
}

glm::mat4 Transform::getModelMatrix() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_Position);
    glm::mat4 rotation = glm::toMat4(m_Rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_Scale);
    return translation * rotation * scale;
}

glm::vec3 Transform::getInterpolatedPosition(float alpha) const {
    if (!m_HasPrevious) {
        return m_Position;
    }
    return m_PreviousPosition + (m_Position - m_PreviousPosition) * alpha;
}

void Transform::updatePrevious() {
    m_PreviousPosition = m_Position;
    m_HasPrevious = true;
}

void Transform::setPosition(const glm::vec3& position) {
    m_Position = position;
}

void Transform::setRotation(const glm::quat& rotation) {
    m_Rotation = glm::normalize(rotation);
}

void Transform::setScale(const glm::vec3& scale) {
    m_Scale = scale;
}

void Transform::translate(const glm::vec3& offset) {
    m_Position += offset;
}

void Transform::rotate(const glm::quat& deltaRotation) {
    m_Rotation = glm::normalize(deltaRotation * m_Rotation);
}

} // namespace PoorCraft

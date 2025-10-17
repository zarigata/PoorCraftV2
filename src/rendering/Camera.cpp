#include "poorcraft/rendering/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
constexpr float DEFAULT_FOV = 90.0f;
constexpr float DEFAULT_ASPECT = 16.0f / 9.0f;
constexpr float DEFAULT_NEAR = 0.1f;
constexpr float DEFAULT_FAR = 1000.0f;
}

Camera::Camera(CameraType type,
               const glm::vec3& position,
               const glm::vec3& target,
               const glm::vec3& up)
    : m_Type(type),
      m_Position(position),
      m_Target(target),
      m_Up(glm::normalize(up)),
      m_ViewMatrix(1.0f),
      m_ProjectionMatrix(1.0f),
      m_FOV(DEFAULT_FOV),
      m_AspectRatio(DEFAULT_ASPECT),
      m_NearPlane(DEFAULT_NEAR),
      m_FarPlane(DEFAULT_FAR) {
    updateViewMatrix();
    if (m_Type == CameraType::PERSPECTIVE) {
        updateProjectionMatrix(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
    }
}

void Camera::updateViewMatrix() {
    m_ViewMatrix = glm::lookAt(m_Position, m_Target, m_Up);
    PC_TRACE("Camera view matrix updated");
}

void Camera::updateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane) {
    m_Type = CameraType::PERSPECTIVE;
    m_FOV = fov;
    m_AspectRatio = aspect;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;

    m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    PC_TRACE("Camera perspective projection updated");
}

void Camera::updateProjectionMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_Type = CameraType::ORTHOGRAPHIC;
    m_ProjectionMatrix = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    PC_TRACE("Camera orthographic projection updated");
}

void Camera::setPosition(const glm::vec3& position) {
    m_Position = position;
    updateViewMatrix();
}

void Camera::setTarget(const glm::vec3& target) {
    m_Target = target;
    updateViewMatrix();
}

void Camera::setUp(const glm::vec3& up) {
    m_Up = glm::normalize(up);
    updateViewMatrix();
}

void Camera::lookAt(const glm::vec3& target) {
    setTarget(target);
}

void Camera::move(const glm::vec3& offset) {
    m_Position += offset;
    m_Target += offset;
    updateViewMatrix();
}

void Camera::rotate(float yawRadians, float pitchRadians) {
    glm::vec3 forward;
    forward.x = cosf(pitchRadians) * cosf(yawRadians);
    forward.y = sinf(pitchRadians);
    forward.z = cosf(pitchRadians) * sinf(yawRadians);
    forward = glm::normalize(forward);

    m_Target = m_Position + forward;
    updateViewMatrix();
}

const glm::mat4& Camera::getViewMatrix() const {
    return m_ViewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    return m_ProjectionMatrix;
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    return m_ProjectionMatrix * m_ViewMatrix;
}

const glm::vec3& Camera::getPosition() const {
    return m_Position;
}

const glm::vec3& Camera::getTarget() const {
    return m_Target;
}

glm::vec3 Camera::getForward() const {
    return glm::normalize(m_Target - m_Position);
}

glm::vec3 Camera::getRight() const {
    return glm::normalize(glm::cross(getForward(), m_Up));
}

const glm::vec3& Camera::getUp() const {
    return m_Up;
}

float Camera::getFOV() const {
    return m_FOV;
}

float Camera::getAspectRatio() const {
    return m_AspectRatio;
}

float Camera::getNearPlane() const {
    return m_NearPlane;
}

float Camera::getFarPlane() const {
    return m_FarPlane;
}

void Camera::onWindowResize(int width, int height) {
    if (height == 0) {
        return;
    }
    m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
    if (m_Type == CameraType::PERSPECTIVE) {
        updateProjectionMatrix(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
    } else {
        PC_DEBUG("Camera window resize ignored for orthographic camera - call orthographic update explicitly");
    }
}

} // namespace PoorCraft

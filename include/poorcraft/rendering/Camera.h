#pragma once

#include <glm/glm.hpp>

namespace PoorCraft {

enum class CameraType {
    PERSPECTIVE,
    ORTHOGRAPHIC
};

class Camera {
public:
    Camera(CameraType type,
           const glm::vec3& position,
           const glm::vec3& target,
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = default;
    Camera& operator=(Camera&&) = default;

    void updateViewMatrix();
    void updateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane);
    void updateProjectionMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    void setPosition(const glm::vec3& position);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);

    void lookAt(const glm::vec3& target);
    void move(const glm::vec3& offset);
    void rotate(float yawRadians, float pitchRadians);

    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;

    const glm::vec3& getPosition() const;
    const glm::vec3& getTarget() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    const glm::vec3& getUp() const;

    float getFOV() const;
    float getAspectRatio() const;
    float getNearPlane() const;
    float getFarPlane() const;

    void onWindowResize(int width, int height);

private:
    CameraType m_Type;
    glm::vec3 m_Position;
    glm::vec3 m_Target;
    glm::vec3 m_Up;

    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;

    float m_FOV = 90.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 1000.0f;
};

} // namespace PoorCraft

#pragma once

#include <array>

#include <glm/glm.hpp>

namespace PoorCraft {
class PhysicsAABB {
public:
    PhysicsAABB() = default;
    PhysicsAABB(const glm::vec3& minCorner, const glm::vec3& maxCorner);

    static PhysicsAABB fromCenterExtents(const glm::vec3& center, const glm::vec3& extents);

    [[nodiscard]] glm::vec3 getCenter() const;
    [[nodiscard]] glm::vec3 getExtents() const;
    [[nodiscard]] const glm::vec3& getMin() const;
    [[nodiscard]] const glm::vec3& getMax() const;

    [[nodiscard]] bool intersects(const PhysicsAABB& other) const;
    [[nodiscard]] bool contains(const glm::vec3& point) const;

    void expand(float amount);
    void translate(const glm::vec3& offset);

    [[nodiscard]] std::array<glm::vec3, 8> getCorners() const;
    [[nodiscard]] PhysicsAABB merge(const PhysicsAABB& other) const;

private:
    glm::vec3 m_Min{0.0f};
    glm::vec3 m_Max{0.0f};
};

} // namespace PoorCraft

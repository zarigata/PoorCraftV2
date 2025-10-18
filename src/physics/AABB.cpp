#include "poorcraft/physics/AABB.h"

#include <algorithm>

namespace PoorCraft {

PhysicsAABB::PhysicsAABB(const glm::vec3& minCorner, const glm::vec3& maxCorner)
    : m_Min(minCorner), m_Max(maxCorner) {
}

PhysicsAABB PhysicsAABB::fromCenterExtents(const glm::vec3& center, const glm::vec3& extents) {
    return PhysicsAABB(center - extents, center + extents);
}

glm::vec3 PhysicsAABB::getCenter() const {
    return (m_Min + m_Max) * 0.5f;
}

glm::vec3 PhysicsAABB::getExtents() const {
    return (m_Max - m_Min) * 0.5f;
}

const glm::vec3& PhysicsAABB::getMin() const {
    return m_Min;
}

const glm::vec3& PhysicsAABB::getMax() const {
    return m_Max;
}

bool PhysicsAABB::intersects(const PhysicsAABB& other) const {
    return (m_Min.x <= other.m_Max.x && m_Max.x >= other.m_Min.x) &&
           (m_Min.y <= other.m_Max.y && m_Max.y >= other.m_Min.y) &&
           (m_Min.z <= other.m_Max.z && m_Max.z >= other.m_Min.z);
}

bool PhysicsAABB::contains(const glm::vec3& point) const {
    return (point.x >= m_Min.x && point.x <= m_Max.x) &&
           (point.y >= m_Min.y && point.y <= m_Max.y) &&
           (point.z >= m_Min.z && point.z <= m_Max.z);
}

void PhysicsAABB::expand(float amount) {
    m_Min -= glm::vec3(amount);
    m_Max += glm::vec3(amount);
}

void PhysicsAABB::translate(const glm::vec3& offset) {
    m_Min += offset;
    m_Max += offset;
}

std::array<glm::vec3, 8> PhysicsAABB::getCorners() const {
    return {
        glm::vec3(m_Min.x, m_Min.y, m_Min.z),
        glm::vec3(m_Max.x, m_Min.y, m_Min.z),
        glm::vec3(m_Min.x, m_Max.y, m_Min.z),
        glm::vec3(m_Max.x, m_Max.y, m_Min.z),
        glm::vec3(m_Min.x, m_Min.y, m_Max.z),
        glm::vec3(m_Max.x, m_Min.y, m_Max.z),
        glm::vec3(m_Min.x, m_Max.y, m_Max.z),
        glm::vec3(m_Max.x, m_Max.y, m_Max.z)};
}

PhysicsAABB PhysicsAABB::merge(const PhysicsAABB& other) const {
    glm::vec3 minCorner(std::min(m_Min.x, other.m_Min.x),
                        std::min(m_Min.y, other.m_Min.y),
                        std::min(m_Min.z, other.m_Min.z));
    glm::vec3 maxCorner(std::max(m_Max.x, other.m_Max.x),
                        std::max(m_Max.y, other.m_Max.y),
                        std::max(m_Max.z, other.m_Max.z));
    return PhysicsAABB(minCorner, maxCorner);
}

} // namespace PoorCraft

#include "poorcraft/world/Frustum.h"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace PoorCraft {

float Plane::getSignedDistance(const glm::vec3& point) const {
    return glm::dot(normal, point) + distance;
}

glm::vec3 AABB::getCenter() const {
    return (min + max) * 0.5f;
}

glm::vec3 AABB::getExtents() const {
    return (max - min) * 0.5f;
}

bool AABB::contains(const glm::vec3& point) const {
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool AABB::intersects(const AABB& other) const {
    return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y &&
           max.y >= other.min.y && min.z <= other.max.z && max.z >= other.min.z;
}

Frustum::Frustum(const glm::mat4& viewProjection) {
    update(viewProjection);
}

void Frustum::update(const glm::mat4& viewProjection) {
    const glm::mat4 transpose = glm::transpose(viewProjection);

    extractPlane(0,
                 transpose[3][0] + transpose[0][0],
                 transpose[3][1] + transpose[0][1],
                 transpose[3][2] + transpose[0][2],
                 transpose[3][3] + transpose[0][3]);
    extractPlane(1,
                 transpose[3][0] - transpose[0][0],
                 transpose[3][1] - transpose[0][1],
                 transpose[3][2] - transpose[0][2],
                 transpose[3][3] - transpose[0][3]);
    extractPlane(2,
                 transpose[3][0] + transpose[1][0],
                 transpose[3][1] + transpose[1][1],
                 transpose[3][2] + transpose[1][2],
                 transpose[3][3] + transpose[1][3]);
    extractPlane(3,
                 transpose[3][0] - transpose[1][0],
                 transpose[3][1] - transpose[1][1],
                 transpose[3][2] - transpose[1][2],
                 transpose[3][3] - transpose[1][3]);
    extractPlane(4,
                 transpose[3][0] + transpose[2][0],
                 transpose[3][1] + transpose[2][1],
                 transpose[3][2] + transpose[2][2],
                 transpose[3][3] + transpose[2][3]);
    extractPlane(5,
                 transpose[3][0] - transpose[2][0],
                 transpose[3][1] - transpose[2][1],
                 transpose[3][2] - transpose[2][2],
                 transpose[3][3] - transpose[2][3]);
}

bool Frustum::containsPoint(const glm::vec3& point) const {
    for (const auto& plane : planes) {
        if (plane.getSignedDistance(point) < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Frustum::containsAABB(const AABB& bounds) const {
    const glm::vec3 center = bounds.getCenter();
    const glm::vec3 extents = bounds.getExtents();

    for (const auto& plane : planes) {
        const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
                        extents.z * std::abs(plane.normal.z);
        const float distance = plane.getSignedDistance(center);

        if (distance + r < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Frustum::containsSphere(const glm::vec3& center, float radius) const {
    for (const auto& plane : planes) {
        if (plane.getSignedDistance(center) < -radius) {
            return false;
        }
    }

    return true;
}

const Plane& Frustum::getPlane(PlaneIndex index) const {
    return planes[static_cast<std::size_t>(index)];
}

void Frustum::extractPlane(std::size_t index, float a, float b, float c, float d) {
    Plane plane{};
    plane.normal = glm::vec3(a, b, c);
    plane.distance = d;
    normalizePlane(plane);
    planes[index] = plane;
}

void Frustum::normalizePlane(Plane& plane) {
    const float length = glm::length(plane.normal);
    if (length == 0.0f) {
        return;
    }

    plane.normal /= length;
    plane.distance /= length;
}

} // namespace PoorCraft

#pragma once

#include <array>

#include <glm/glm.hpp>

namespace PoorCraft {

struct Plane {
    glm::vec3 normal;
    float distance;

    [[nodiscard]] float getSignedDistance(const glm::vec3& point) const;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    [[nodiscard]] glm::vec3 getCenter() const;
    [[nodiscard]] glm::vec3 getExtents() const;
    [[nodiscard]] bool contains(const glm::vec3& point) const;
    [[nodiscard]] bool intersects(const AABB& other) const;
};

class Frustum {
public:
    enum class PlaneIndex : uint8_t {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5
    };

    Frustum() = default;
    explicit Frustum(const glm::mat4& viewProjection);

    void update(const glm::mat4& viewProjection);

    [[nodiscard]] bool containsPoint(const glm::vec3& point) const;
    [[nodiscard]] bool containsAABB(const AABB& bounds) const;
    [[nodiscard]] bool containsSphere(const glm::vec3& center, float radius) const;

    [[nodiscard]] const Plane& getPlane(PlaneIndex index) const;

private:
    void extractPlane(std::size_t index, float a, float b, float c, float d);
    void normalizePlane(Plane& plane);

    std::array<Plane, 6> planes{};
};

} // namespace PoorCraft

#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "poorcraft/physics/RaycastHit.h"

namespace PoorCraft {

class PhysicsWorld;

class Raycaster {
public:
    static RaycastHit raycast(const glm::vec3& origin,
                              const glm::vec3& direction,
                              float maxDistance,
                              const PhysicsWorld& physicsWorld);

    static std::vector<glm::ivec3> raycastBlocks(const glm::vec3& origin,
                                                 const glm::vec3& direction,
                                                 float maxDistance,
                                                 const PhysicsWorld& physicsWorld);

private:
    static int sign(float value);
    static float intbound(float s, float ds);
};

} // namespace PoorCraft

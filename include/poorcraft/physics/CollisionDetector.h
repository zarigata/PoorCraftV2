#pragma once

#include <glm/glm.hpp>

#include "poorcraft/physics/AABB.h"

namespace PoorCraft {

class PhysicsWorld;

struct CollisionResult {
    bool collided = false;
    glm::vec3 normal{0.0f};
    float penetration = 0.0f;
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
};

class CollisionDetector {
public:
    static CollisionResult sweepAABB(const PhysicsAABB& bounds,
                                     const glm::vec3& displacement,
                                     PhysicsWorld& physicsWorld);

    static CollisionResult resolveCollision(const PhysicsAABB& bounds,
                                            const glm::vec3& displacement,
                                            PhysicsWorld& physicsWorld);

    static bool checkGrounded(const PhysicsAABB& bounds,
                              const glm::vec3& velocity,
                              PhysicsWorld& physicsWorld);

    static bool stepUp(PhysicsAABB& bounds,
                       glm::vec3& velocity,
                       PhysicsWorld& physicsWorld,
                       float maxStepHeight);

private:
    static glm::vec3 clipVelocity(const glm::vec3& velocity, const glm::vec3& normal, float overbounce);
};

} // namespace PoorCraft

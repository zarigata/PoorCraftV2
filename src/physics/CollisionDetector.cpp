#include "poorcraft/physics/CollisionDetector.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "poorcraft/core/Logger.h"
#include "poorcraft/physics/PhysicsWorld.h"

namespace PoorCraft {

namespace {
constexpr int MAX_ITERATIONS = 3;
constexpr float GROUND_CHECK_DEPTH = 0.05f;
constexpr float OVERBOUNCE = 1.001f;
constexpr float GROUND_VELOCITY_EPS = 0.5f;
constexpr float TOI_EPSILON = 1e-4f;
constexpr float MIN_DISPLACEMENT = 1e-4f;
} // namespace

CollisionResult CollisionDetector::sweepAABB(const PhysicsAABB& bounds,
                                             const glm::vec3& displacement,
                                             PhysicsWorld& physicsWorld) {
    CollisionResult result;
    result.velocity = displacement;

    if (glm::length2(displacement) <= 0.0f) {
        result.position = bounds.getCenter();
        return result;
    }

    glm::vec3 remaining = displacement;
    glm::vec3 currentCenter = bounds.getCenter();
    const glm::vec3 extents = bounds.getExtents();

    const float minDisplacementSq = MIN_DISPLACEMENT * MIN_DISPLACEMENT;

    for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
        if (glm::length2(remaining) < minDisplacementSq) {
            PhysicsAABB moved = PhysicsAABB::fromCenterExtents(currentCenter + remaining, extents);
            auto solids = physicsWorld.getSurroundingBlocks(moved);

            float earliestPenetration = std::numeric_limits<float>::max();
            glm::vec3 bestNormal(0.0f);
            glm::vec3 resolvedPosition = moved.getCenter();

            for (const auto& blockPos : solids) {
                PhysicsAABB blockAABB = physicsWorld.getBlockAABB(blockPos.x, blockPos.y, blockPos.z);
                if (!moved.intersects(blockAABB)) {
                    continue;
                }

                float penetration = 0.0f;
                glm::vec3 normal = getCollisionNormal(moved, blockAABB, penetration);
                if (penetration < earliestPenetration) {
                    earliestPenetration = penetration;
                    bestNormal = normal;
                    resolvedPosition = moved.getCenter() + normal * penetration;
                }
            }

            if (earliestPenetration < std::numeric_limits<float>::max()) {
                result.collided = true;
                result.normal = bestNormal;
                result.penetration = earliestPenetration;
                result.position = resolvedPosition;
                result.velocity = clipVelocity(remaining, bestNormal, OVERBOUNCE);
            } else {
                currentCenter = moved.getCenter();
                result.position = currentCenter;
                result.velocity = remaining;
            }

            return result;
        }

        PhysicsAABB expanded = PhysicsAABB::fromCenterExtents(currentCenter, extents);
        expanded.translate(remaining);
        auto solids = physicsWorld.getSurroundingBlocks(expanded);

        float hitTime = 1.0f;
        glm::vec3 hitNormal(0.0f);
        bool hit = false;

        for (const auto& blockPos : solids) {
            PhysicsAABB blockAABB = physicsWorld.getBlockAABB(blockPos.x, blockPos.y, blockPos.z);

            float enter = 0.0f;
            float exit = 1.0f;
            glm::vec3 normal(0.0f);

            const glm::vec3 invDir = glm::vec3(
                remaining.x != 0.0f ? 1.0f / remaining.x : std::numeric_limits<float>::infinity(),
                remaining.y != 0.0f ? 1.0f / remaining.y : std::numeric_limits<float>::infinity(),
                remaining.z != 0.0f ? 1.0f / remaining.z : std::numeric_limits<float>::infinity());

            const glm::vec3 movingMin = currentCenter - extents;
            const glm::vec3 movingMax = currentCenter + extents;
            const glm::vec3 blockMin = blockAABB.getMin();
            const glm::vec3 blockMax = blockAABB.getMax();

            for (int axis = 0; axis < 3; ++axis) {
                const float min = movingMin[axis];
                const float max = movingMax[axis];
                const float blockMinAxis = blockMin[axis];
                const float blockMaxAxis = blockMax[axis];
                const float dir = remaining[axis];

                if (std::abs(dir) < TOI_EPSILON) {
                    if (max <= blockMinAxis || min >= blockMaxAxis) {
                        enter = 1.0f;
                        exit = 0.0f;
                        break;
                    }
                    continue;
                }

                const float t1 = (blockMinAxis - max) * invDir[axis];
                const float t2 = (blockMaxAxis - min) * invDir[axis];

                float slabEnter = std::min(t1, t2);
                float slabExit = std::max(t1, t2);

                if (slabEnter > enter) {
                    enter = slabEnter;
                    normal = glm::vec3(0.0f);
                    normal[axis] = dir > 0.0f ? -1.0f : 1.0f;
                }
                exit = std::min(exit, slabExit);

                if (enter > exit || exit < 0.0f || enter > 1.0f) {
                    break;
                }
            }

            if (enter <= exit && enter >= 0.0f && enter < hitTime) {
                hitTime = enter;
                hitNormal = normal;
                hit = true;
            }
        }

        if (!hit) {
            currentCenter += remaining;
            result.position = currentCenter;
            result.velocity = remaining;
            return result;
        }

        const glm::vec3 move = remaining * std::max(hitTime - TOI_EPSILON, 0.0f);
        currentCenter += move;

        result.collided = true;
        result.normal = hitNormal;
        result.position = currentCenter;

        glm::vec3 remainingAfterHit = remaining - move;
        remainingAfterHit -= hitNormal * glm::dot(remainingAfterHit, hitNormal);

        remaining = clipVelocity(remainingAfterHit, hitNormal, OVERBOUNCE);
    }

    result.position = currentCenter;
    result.velocity = glm::vec3(0.0f);
    return result;
}

CollisionResult CollisionDetector::resolveCollision(const PhysicsAABB& bounds,
                                                    const glm::vec3& displacement,
                                                    PhysicsWorld& physicsWorld) {
    CollisionResult finalResult;
    finalResult.velocity = displacement;

    glm::vec3 remainingVelocity = displacement;
    PhysicsAABB current = bounds;

    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        if (glm::length2(remainingVelocity) <= 0.0f) {
            break;
        }

        CollisionResult iterationResult = sweepAABB(current, remainingVelocity, physicsWorld);
        if (!iterationResult.collided) {
            current.translate(remainingVelocity);
            finalResult.position = current.getCenter();
            finalResult.velocity = remainingVelocity;
            return finalResult;
        }

        current = PhysicsAABB::fromCenterExtents(iterationResult.position, current.getExtents());
        remainingVelocity = iterationResult.velocity;
        finalResult = iterationResult;
    }

    finalResult.position = current.getCenter();
    return finalResult;
}

bool CollisionDetector::checkGrounded(const PhysicsAABB& bounds,
                                      const glm::vec3& velocity,
                                      PhysicsWorld& physicsWorld) {
    if (velocity.y > GROUND_VELOCITY_EPS) {
        return false;
    }

    PhysicsAABB probe = bounds;
    probe.translate(glm::vec3(0.0f, -GROUND_CHECK_DEPTH, 0.0f));

    auto solids = physicsWorld.getSurroundingBlocks(probe);
    for (const auto& blockPos : solids) {
        PhysicsAABB blockAABB = physicsWorld.getBlockAABB(blockPos.x, blockPos.y, blockPos.z);
        if (probe.intersects(blockAABB)) {
            return true;
        }
    }

    return false;
}

bool CollisionDetector::stepUp(PhysicsAABB& bounds,
                               glm::vec3& velocity,
                               PhysicsWorld& physicsWorld,
                               float maxStepHeight) {
    if (std::abs(velocity.y) > 0.0001f) {
        return false;
    }

    const float stepIncrement = 0.1f;
    for (float step = stepIncrement; step <= maxStepHeight; step += stepIncrement) {
        PhysicsAABB raised = bounds;
        raised.translate(glm::vec3(0.0f, step, 0.0f));

        auto solids = physicsWorld.getSurroundingBlocks(raised);
        bool blocked = false;
        for (const auto& blockPos : solids) {
            PhysicsAABB blockAABB = physicsWorld.getBlockAABB(blockPos.x, blockPos.y, blockPos.z);
            if (raised.intersects(blockAABB)) {
                blocked = true;
                break;
            }
        }

        if (!blocked) {
            bounds = raised;
            return true;
        }
    }

    return false;
}

glm::vec3 CollisionDetector::getCollisionNormal(const PhysicsAABB& movingAABB,
                                                const PhysicsAABB& blockAABB,
                                                float& outPenetration) {
    const glm::vec3 movingMin = movingAABB.getMin();
    const glm::vec3 movingMax = movingAABB.getMax();
    const glm::vec3 blockMin = blockAABB.getMin();
    const glm::vec3 blockMax = blockAABB.getMax();

    const float penX1 = blockMax.x - movingMin.x;
    const float penX2 = movingMax.x - blockMin.x;
    const float penY1 = blockMax.y - movingMin.y;
    const float penY2 = movingMax.y - blockMin.y;
    const float penZ1 = blockMax.z - movingMin.z;
    const float penZ2 = movingMax.z - blockMin.z;

    float minPenetration = std::min({penX1, penX2, penY1, penY2, penZ1, penZ2});
    outPenetration = minPenetration;

    if (minPenetration == penX1) {
        return glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (minPenetration == penX2) {
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (minPenetration == penY1) {
        return glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (minPenetration == penY2) {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (minPenetration == penZ1) {
        return glm::vec3(0.0f, 0.0f, -1.0f);
    }
    return glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 CollisionDetector::clipVelocity(const glm::vec3& velocity,
                                          const glm::vec3& normal,
                                          float overbounce) {
    const float backoff = glm::dot(velocity, normal) * overbounce;
    glm::vec3 clipped = velocity - normal * backoff;

    if (glm::dot(clipped, clipped) < 0.0001f) {
        clipped = glm::vec3(0.0f);
    }

    return clipped;
}

} // namespace PoorCraft

#include "poorcraft/physics/CollisionDetector.h"

#include <algorithm>

#include "poorcraft/core/Logger.h"
#include "poorcraft/physics/PhysicsWorld.h"

namespace PoorCraft {

namespace {
constexpr int MAX_ITERATIONS = 3;
constexpr float GROUND_CHECK_DEPTH = 0.1f;
constexpr float OVERBOUNCE = 1.001f;
} // namespace

CollisionResult CollisionDetector::sweepAABB(const PhysicsAABB& bounds,
                                             const glm::vec3& velocity,
                                             PhysicsWorld& physicsWorld) {
    CollisionResult result;
    result.velocity = velocity;

    if (glm::length2(velocity) == 0.0f) {
        result.position = bounds.getCenter();
        return result;
    }

    PhysicsAABB moved = PhysicsAABB::fromCenterExtents(bounds.getCenter() + velocity, bounds.getExtents());
    auto solids = physicsWorld.getSurroundingBlocks(moved);

    float earliestPenetration = std::numeric_limits<float>::max();
    glm::vec3 bestNormal{0.0f};
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
        result.velocity = clipVelocity(velocity, bestNormal, OVERBOUNCE);
    } else {
        result.position = moved.getCenter();
    }

    return result;
}

CollisionResult CollisionDetector::resolveCollision(const PhysicsAABB& bounds,
                                                    const glm::vec3& velocity,
                                                    PhysicsWorld& physicsWorld) {
    CollisionResult finalResult;
    finalResult.velocity = velocity;

    glm::vec3 remainingVelocity = velocity;
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

bool CollisionDetector::checkGrounded(const PhysicsAABB& bounds, PhysicsWorld& physicsWorld) {
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

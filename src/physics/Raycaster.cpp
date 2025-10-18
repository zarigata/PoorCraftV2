#include "poorcraft/physics/Raycaster.h"

#include <cmath>
#include <limits>

#include "poorcraft/physics/PhysicsWorld.h"

namespace PoorCraft {
namespace {
constexpr float EPSILON = 1e-6f;

int axisFromStep(const glm::ivec3& step) {
    if (step.x != 0) {
        return 0;
    }
    if (step.y != 0) {
        return 1;
    }
    return 2;
}

BlockFace faceFromStep(int axis, int step) {
    switch (axis) {
    case 0:
        return step > 0 ? BlockFace::LEFT : BlockFace::RIGHT;
    case 1:
        return step > 0 ? BlockFace::BOTTOM : BlockFace::TOP;
    default:
        return step > 0 ? BlockFace::BACK : BlockFace::FRONT;
    }
}

} // namespace

int Raycaster::sign(float value) {
    if (value > 0.0f) {
        return 1;
    }
    if (value < 0.0f) {
        return -1;
    }
    return 0;
}

float Raycaster::intbound(float s, float ds) {
    if (std::abs(ds) <= EPSILON) {
        return std::numeric_limits<float>::infinity();
    }

    float sFraction = s - std::floor(s);
    if (ds < 0.0f) {
        sFraction = 1.0f - sFraction;
    }

    return sFraction / std::abs(ds);
}

RaycastHit Raycaster::raycast(const glm::vec3& origin,
                              const glm::vec3& direction,
                              float maxDistance,
                              const PhysicsWorld& physicsWorld) {
    RaycastHit result;

    if (maxDistance <= 0.0f) {
        return result;
    }

    glm::vec3 dir = direction;
    if (glm::length2(dir) <= EPSILON) {
        return result;
    }
    dir = glm::normalize(dir);

    glm::vec3 position = origin;
    glm::ivec3 currentCell = glm::floor(position);

    glm::ivec3 step(sign(dir.x), sign(dir.y), sign(dir.z));

    glm::vec3 tMax(intbound(position.x, dir.x),
                   intbound(position.y, dir.y),
                   intbound(position.z, dir.z));

    glm::vec3 tDelta((step.x == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.x),
                     (step.y == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.y),
                     (step.z == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.z));

    if (physicsWorld.isBlockSolid(static_cast<float>(currentCell.x),
                                  static_cast<float>(currentCell.y),
                                  static_cast<float>(currentCell.z))) {
        result.hit = true;
        result.blockPos = currentCell;
        result.position = position;
        result.distance = 0.0f;
        result.normal = glm::vec3(0.0f);
        result.face = BlockFace::FRONT;
        return result;
    }

    float distanceTravelled = 0.0f;
    while (distanceTravelled <= maxDistance) {
        int axis = 0;
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                axis = 0;
            } else {
                axis = 2;
            }
        } else {
            if (tMax.y < tMax.z) {
                axis = 1;
            } else {
                axis = 2;
            }
        }

        distanceTravelled = tMax[axis];
        if (distanceTravelled > maxDistance) {
            break;
        }

        currentCell[axis] += step[axis];
        tMax[axis] += tDelta[axis];

        if (physicsWorld.isBlockSolid(static_cast<float>(currentCell.x),
                                      static_cast<float>(currentCell.y),
                                      static_cast<float>(currentCell.z))) {
            result.hit = true;
            result.blockPos = currentCell;
            result.distance = distanceTravelled;
            result.position = origin + dir * distanceTravelled;

            glm::vec3 normal(0.0f);
            normal[axis] = static_cast<float>(-step[axis]);
            result.normal = normal;
            result.face = faceFromStep(axis, step[axis]);
            return result;
        }
    }

    return result;
}

std::vector<glm::ivec3> Raycaster::raycastBlocks(const glm::vec3& origin,
                                                 const glm::vec3& direction,
                                                 float maxDistance,
                                                 const PhysicsWorld& physicsWorld) {
    std::vector<glm::ivec3> visited;

    if (maxDistance <= 0.0f) {
        return visited;
    }

    glm::vec3 dir = direction;
    if (glm::length2(dir) <= EPSILON) {
        return visited;
    }
    dir = glm::normalize(dir);

    glm::vec3 position = origin;
    glm::ivec3 currentCell = glm::floor(position);

    glm::ivec3 step(sign(dir.x), sign(dir.y), sign(dir.z));

    glm::vec3 tMax(intbound(position.x, dir.x),
                   intbound(position.y, dir.y),
                   intbound(position.z, dir.z));

    glm::vec3 tDelta((step.x == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.x),
                     (step.y == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.y),
                     (step.z == 0) ? std::numeric_limits<float>::infinity() : std::abs(1.0f / dir.z));

    float distanceTravelled = 0.0f;
    while (distanceTravelled <= maxDistance) {
        visited.push_back(currentCell);

        int axis = 0;
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                axis = 0;
            } else {
                axis = 2;
            }
        } else {
            if (tMax.y < tMax.z) {
                axis = 1;
            } else {
                axis = 2;
            }
        }

        distanceTravelled = tMax[axis];
        if (distanceTravelled > maxDistance) {
            break;
        }

        currentCell[axis] += step[axis];
        tMax[axis] += tDelta[axis];
    }

    return visited;
}

} // namespace PoorCraft

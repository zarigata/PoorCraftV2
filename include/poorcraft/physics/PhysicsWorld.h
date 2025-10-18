#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "poorcraft/physics/AABB.h"
#include "poorcraft/physics/RaycastHit.h"
namespace PoorCraft {

class World;

class PhysicsWorld {
public:
    explicit PhysicsWorld(World& world) noexcept;

    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;
    PhysicsWorld(PhysicsWorld&&) = delete;
    PhysicsWorld& operator=(PhysicsWorld&&) = delete;

    [[nodiscard]] uint16_t getBlockAt(float worldX, float worldY, float worldZ) const;
    [[nodiscard]] bool isBlockSolid(float worldX, float worldY, float worldZ) const;
    [[nodiscard]] PhysicsAABB getBlockAABB(int worldX, int worldY, int worldZ) const;
    [[nodiscard]] std::vector<glm::ivec3> getSurroundingBlocks(const PhysicsAABB& bounds) const;

private:
    [[nodiscard]] glm::ivec3 worldToBlockCoords(const glm::vec3& worldPos) const;
    [[nodiscard]] uint16_t getBlockAt(const glm::ivec3& blockPos) const;

    World& m_World;
};

} // namespace PoorCraft

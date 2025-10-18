#include "poorcraft/physics/PhysicsWorld.h"

#include <cmath>
#include <limits>

#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkCoord.h"
#include "poorcraft/world/ChunkManager.h"
#include "poorcraft/world/World.h"
#include "poorcraft/world/BlockType.h"

namespace PoorCraft {

namespace {
constexpr float EPSILON = 1e-5f;
constexpr uint16_t BLOCK_ID_AIR = 0;
} // namespace

PhysicsWorld::PhysicsWorld(World& world) noexcept
    : m_World(world) {
}

uint16_t PhysicsWorld::getBlockAt(float worldX, float worldY, float worldZ) const {
    return getBlockAt(worldToBlockCoords(glm::vec3(worldX, worldY, worldZ)));
}

bool PhysicsWorld::isBlockSolid(float worldX, float worldY, float worldZ) const {
    const uint16_t blockId = getBlockAt(worldX, worldY, worldZ);
    if (blockId == BLOCK_ID_AIR) {
        return false;
    }

    auto& registry = m_World.getBlockRegistry();
    const BlockType& blockType = registry.getBlockType(blockId);
    return blockType.isSolid;
}

PhysicsAABB PhysicsWorld::getBlockAABB(int worldX, int worldY, int worldZ) const {
    const glm::vec3 minCorner(worldX, worldY, worldZ);
    const glm::vec3 maxCorner = minCorner + glm::vec3(1.0f);
    return PhysicsAABB(minCorner, maxCorner);
}

std::vector<glm::ivec3> PhysicsWorld::getSurroundingBlocks(const PhysicsAABB& bounds) const {
    const glm::vec3 minWorld = bounds.getMin() - glm::vec3(EPSILON);
    const glm::vec3 maxWorld = bounds.getMax() + glm::vec3(EPSILON);

    const glm::ivec3 minBlock = worldToBlockCoords(minWorld);
    const glm::ivec3 maxBlock = worldToBlockCoords(maxWorld);

    std::vector<glm::ivec3> blocks;
    blocks.reserve((maxBlock.x - minBlock.x + 1) * (maxBlock.y - minBlock.y + 1) *
                   (maxBlock.z - minBlock.z + 1));

    for (int x = minBlock.x; x <= maxBlock.x; ++x) {
        for (int y = minBlock.y; y <= maxBlock.y; ++y) {
            for (int z = minBlock.z; z <= maxBlock.z; ++z) {
                const glm::ivec3 blockPos(x, y, z);
                if (isBlockSolid(static_cast<float>(blockPos.x),
                                 static_cast<float>(blockPos.y),
                                 static_cast<float>(blockPos.z))) {
                    blocks.push_back(blockPos);
                }
            }
        }
    }

    return blocks;
}

glm::ivec3 PhysicsWorld::worldToBlockCoords(const glm::vec3& worldPos) const {
    return glm::ivec3(static_cast<int>(std::floor(worldPos.x)),
                      static_cast<int>(std::floor(worldPos.y)),
                      static_cast<int>(std::floor(worldPos.z)));
}

uint16_t PhysicsWorld::getBlockAt(const glm::ivec3& blockPos) const {
    auto& chunkManager = m_World.getChunkManager();

    const ChunkCoord chunkCoord = ChunkCoord::fromWorldPos(static_cast<float>(blockPos.x),
                                                           static_cast<float>(blockPos.z));
    Chunk* chunk = chunkManager.getChunk(chunkCoord);
    if (!chunk) {
        return BLOCK_ID_AIR;
    }

    const int32_t localX = blockPos.x - chunkCoord.x * Chunk::CHUNK_SIZE_X;
    const int32_t localY = blockPos.y;
    const int32_t localZ = blockPos.z - chunkCoord.z * Chunk::CHUNK_SIZE_Z;

    if (localY < 0 || localY >= Chunk::CHUNK_SIZE_Y) {
        return BLOCK_ID_AIR;
    }

    return chunk->getBlock(localX, localY, localZ);
}

} // namespace PoorCraft

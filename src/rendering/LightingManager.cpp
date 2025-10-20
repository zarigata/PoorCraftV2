#include "poorcraft/rendering/LightingManager.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkManager.h"

#include <algorithm>
#include <queue>

namespace PoorCraft {

LightingManager::LightingManager(ChunkManager& chunkManager)
    : chunkManager(chunkManager), lightUpdateQueue() {}

void LightingManager::initialize() {
    PC_INFO("LightingManager initialized");
}

void LightingManager::updateLighting(Chunk& chunk) {
    propagateSkyLight(chunk, chunkManager);
    propagateBlockLight(chunk, chunkManager);
    chunk.setDirty(true);
}

void LightingManager::propagateSkyLight(Chunk& chunk, ChunkManager& chunkManager) {
    std::queue<LightNode> lightQueue;

    // Fill top layer with full sunlight
    for (int32_t x = 0; x < Chunk::CHUNK_SIZE_X; ++x) {
        for (int32_t z = 0; z < Chunk::CHUNK_SIZE_Z; ++z) {
            const int32_t topY = Chunk::CHUNK_SIZE_Y - 1;
            if (shouldPropagate(chunk.getBlock(x, topY, z), false)) {
                chunk.setSkyLight(x, topY, z, 15);
                lightQueue.push({x, topY, z, 15, chunk.getPosition()});
            }
        }
    }

    // BFS flood fill
    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        lightQueue.pop();

        if (node.level <= 1) {
            continue;
        }

        const uint8_t newLevel = node.level - 1;

        // Check 6 neighbors
        const int32_t neighbors[6][3] = {
            {node.x + 1, node.y, node.z},
            {node.x - 1, node.y, node.z},
            {node.x, node.y + 1, node.z},
            {node.x, node.y - 1, node.z},
            {node.x, node.y, node.z + 1},
            {node.x, node.y, node.z - 1}
        };

        for (const auto& neighbor : neighbors) {
            int32_t nx = neighbor[0];
            int32_t ny = neighbor[1];
            int32_t nz = neighbor[2];

            // Handle chunk boundaries
            Chunk* targetChunk = &chunk;
            ChunkCoord targetCoord = chunk.getPosition();

            if (nx < 0) {
                targetCoord.x -= 1;
                nx += Chunk::CHUNK_SIZE_X;
                targetChunk = chunkManager.getChunk(targetCoord);
            } else if (nx >= Chunk::CHUNK_SIZE_X) {
                targetCoord.x += 1;
                nx -= Chunk::CHUNK_SIZE_X;
                targetChunk = chunkManager.getChunk(targetCoord);
            }

            if (nz < 0) {
                targetCoord.z -= 1;
                nz += Chunk::CHUNK_SIZE_Z;
                targetChunk = chunkManager.getChunk(targetCoord);
            } else if (nz >= Chunk::CHUNK_SIZE_Z) {
                targetCoord.z += 1;
                nz -= Chunk::CHUNK_SIZE_Z;
                targetChunk = chunkManager.getChunk(targetCoord);
            }

            if (!targetChunk || ny < 0 || ny >= Chunk::CHUNK_SIZE_Y) {
                continue;
            }

            const uint16_t blockId = targetChunk->getBlock(nx, ny, nz);
            if (!shouldPropagate(blockId, false)) {
                continue;
            }

            const uint8_t currentLight = targetChunk->getSkyLight(nx, ny, nz);
            if (newLevel > currentLight) {
                targetChunk->setSkyLight(nx, ny, nz, newLevel);
                lightQueue.push({nx, ny, nz, newLevel, targetCoord});
            }
        }
    }
}

void LightingManager::propagateBlockLight(Chunk& chunk, ChunkManager& chunkManager) {
    std::queue<LightNode> lightQueue;
    const auto& registry = BlockRegistry::getInstance();

    // Find emissive blocks
    for (int32_t x = 0; x < Chunk::CHUNK_SIZE_X; ++x) {
        for (int32_t y = 0; y < Chunk::CHUNK_SIZE_Y; ++y) {
            for (int32_t z = 0; z < Chunk::CHUNK_SIZE_Z; ++z) {
                const uint16_t blockId = chunk.getBlock(x, y, z);
                if (blockId == 0) {
                    continue;
                }

                const BlockType& blockType = registry.getBlock(blockId);
                if (blockType.lightEmission > 0) {
                    const uint8_t lightLevel = blockType.lightEmission;
                    chunk.setBlockLight(x, y, z, lightLevel);
                    lightQueue.push({x, y, z, lightLevel, chunk.getPosition()});
                }
            }
        }
    }

    // BFS flood fill
    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        lightQueue.pop();

        if (node.level <= 1) {
            continue;
        }

        const uint8_t newLevel = node.level - 1;

        // Check 6 neighbors
        const int32_t neighbors[6][3] = {
            {node.x + 1, node.y, node.z},
            {node.x - 1, node.y, node.z},
            {node.x, node.y + 1, node.z},
            {node.x, node.y - 1, node.z},
            {node.x, node.y, node.z + 1},
            {node.x, node.y, node.z - 1}
        };

        for (const auto& neighbor : neighbors) {
            int32_t nx = neighbor[0];
            int32_t ny = neighbor[1];
            int32_t nz = neighbor[2];

            // Handle chunk boundaries
            Chunk* targetChunk = &chunk;
            ChunkCoord targetCoord = chunk.getPosition();

            if (nx < 0) {
                targetCoord.x -= 1;
                nx += Chunk::CHUNK_SIZE_X;
                targetChunk = chunkManager.getChunk(targetCoord);
            } else if (nx >= Chunk::CHUNK_SIZE_X) {
                targetCoord.x += 1;
                nx -= Chunk::CHUNK_SIZE_X;
                targetChunk = chunkManager.getChunk(targetCoord);
            }

            if (nz < 0) {
                targetCoord.z -= 1;
                nz += Chunk::CHUNK_SIZE_Z;
                targetChunk = chunkManager.getChunk(targetCoord);
            } else if (nz >= Chunk::CHUNK_SIZE_Z) {
                targetCoord.z += 1;
                nz -= Chunk::CHUNK_SIZE_Z;
                targetChunk = chunkManager.getChunk(targetCoord);
            }

            if (!targetChunk || ny < 0 || ny >= Chunk::CHUNK_SIZE_Y) {
                continue;
            }

            const uint16_t blockId = targetChunk->getBlock(nx, ny, nz);
            if (!shouldPropagate(blockId, true)) {
                continue;
            }

            const uint8_t currentLight = targetChunk->getBlockLight(nx, ny, nz);
            if (newLevel > currentLight) {
                targetChunk->setBlockLight(nx, ny, nz, newLevel);
                lightQueue.push({nx, ny, nz, newLevel, targetCoord});
            }
        }
    }
}

void LightingManager::floodFillLight(Chunk& chunk,
                                     ChunkManager& chunkManager,
                                     int32_t startX,
                                     int32_t startY,
                                     int32_t startZ,
                                     uint8_t lightLevel,
                                     bool isBlockLight) {
    std::queue<LightNode> lightQueue;
    lightQueue.push({startX, startY, startZ, lightLevel, chunk.getPosition()});

    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        lightQueue.pop();

        if (node.level <= 1) {
            continue;
        }

        const uint8_t newLevel = node.level - 1;

        // Check 6 neighbors
        const int32_t neighbors[6][3] = {
            {node.x + 1, node.y, node.z},
            {node.x - 1, node.y, node.z},
            {node.x, node.y + 1, node.z},
            {node.x, node.y - 1, node.z},
            {node.x, node.y, node.z + 1},
            {node.x, node.y, node.z - 1}
        };

        for (const auto& neighbor : neighbors) {
            int32_t nx = neighbor[0];
            int32_t ny = neighbor[1];
            int32_t nz = neighbor[2];

            Chunk* targetChunk = &chunk;
            ChunkCoord targetCoord = chunk.getPosition();

            if (nx < 0 || nx >= Chunk::CHUNK_SIZE_X ||
                nz < 0 || nz >= Chunk::CHUNK_SIZE_Z) {
                continue;
            }

            if (ny < 0 || ny >= Chunk::CHUNK_SIZE_Y) {
                continue;
            }

            const uint16_t blockId = targetChunk->getBlock(nx, ny, nz);
            if (!shouldPropagate(blockId, isBlockLight)) {
                continue;
            }

            const uint8_t currentLight = isBlockLight
                                             ? targetChunk->getBlockLight(nx, ny, nz)
                                             : targetChunk->getSkyLight(nx, ny, nz);

            if (newLevel > currentLight) {
                if (isBlockLight) {
                    targetChunk->setBlockLight(nx, ny, nz, newLevel);
                } else {
                    targetChunk->setSkyLight(nx, ny, nz, newLevel);
                }
                lightQueue.push({nx, ny, nz, newLevel, targetCoord});
            }
        }
    }
}

bool LightingManager::shouldPropagate(uint16_t blockId, bool isBlockLight) const {
    if (blockId == 0) {
        return true; // Air allows light propagation
    }

    const auto& registry = BlockRegistry::getInstance();
    const BlockType& blockType = registry.getBlock(blockId);

    // Transparent blocks allow light propagation
    return !blockType.isOpaque;
}

void LightingManager::updateChunkLighting(const ChunkCoord& chunkCoord) {
    Chunk* chunk = chunkManager.getChunk(chunkCoord);
    if (!chunk) {
        return;
    }

    updateLighting(*chunk);

    // Mark neighbor chunks dirty if light crosses boundaries
    const ChunkCoord neighbors[4] = {
        {chunkCoord.x + 1, chunkCoord.z},
        {chunkCoord.x - 1, chunkCoord.z},
        {chunkCoord.x, chunkCoord.z + 1},
        {chunkCoord.x, chunkCoord.z - 1}
    };

    for (const auto& neighborCoord : neighbors) {
        Chunk* neighborChunk = chunkManager.getChunk(neighborCoord);
        if (neighborChunk) {
            neighborChunk->setDirty(true);
        }
    }

    PC_DEBUG("Updated lighting for chunk at " + chunkCoord.toString());
}

void LightingManager::recalculateAllLighting() {
    PC_INFO("Recalculating lighting for all loaded chunks...");

    const auto& chunks = chunkManager.getChunks();
    size_t processedCount = 0;

    for (const auto& [coord, chunk] : chunks) {
        if (chunk) {
            updateLighting(*chunk);
            ++processedCount;

            if (processedCount % 100 == 0) {
                PC_INFO("Processed " + std::to_string(processedCount) + " chunks...");
            }
        }
    }

    PC_INFO("Lighting recalculation complete. Processed " + std::to_string(processedCount) + " chunks.");
}

} // namespace PoorCraft

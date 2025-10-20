#pragma once

#include "poorcraft/world/ChunkCoord.h"

#include <memory>
#include <queue>

namespace PoorCraft {

class ChunkManager;
class Chunk;

class LightingManager {
public:
    explicit LightingManager(ChunkManager& chunkManager);
    LightingManager(const LightingManager&) = delete;
    LightingManager& operator=(const LightingManager&) = delete;
    LightingManager(LightingManager&&) = delete;
    LightingManager& operator=(LightingManager&&) = delete;
    ~LightingManager() = default;

    void initialize();
    void updateLighting(Chunk& chunk);
    void propagateSkyLight(Chunk& chunk, ChunkManager& chunkManager);
    void propagateBlockLight(Chunk& chunk, ChunkManager& chunkManager);
    void updateChunkLighting(const ChunkCoord& chunkCoord);
    void recalculateAllLighting();

private:
    struct LightNode {
        int32_t x, y, z;
        uint8_t level;
        ChunkCoord chunkCoord;
    };

    void floodFillLight(Chunk& chunk,
                        ChunkManager& chunkManager,
                        int32_t startX,
                        int32_t startY,
                        int32_t startZ,
                        uint8_t lightLevel,
                        bool isBlockLight);

    bool shouldPropagate(uint16_t blockId, bool isBlockLight) const;

    ChunkManager& chunkManager;
    std::queue<ChunkCoord> lightUpdateQueue;
};

} // namespace PoorCraft

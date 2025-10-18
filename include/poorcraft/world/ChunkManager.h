#pragma once

#include "poorcraft/rendering/TextureAtlas.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkCoord.h"
#include "poorcraft/world/ChunkMesh.h"

#include <glm/vec3.hpp>

#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace PoorCraft {

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    void initialize();
    void shutdown();

    void update(const glm::vec3& cameraPosition, int renderDistance);

    Chunk* getChunk(const ChunkCoord& coord) const;
    bool hasChunk(const ChunkCoord& coord) const;
    Chunk& getOrCreateChunk(const ChunkCoord& coord);

    void unloadChunk(const ChunkCoord& coord);

    std::size_t getLoadedChunkCount() const;

    ChunkMesh* getChunkMesh(const ChunkCoord& coord) const;

    void setTextureAtlas(TextureAtlas* atlasPtr);

    [[nodiscard]] const std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash>& getChunks() const;
    [[nodiscard]] const std::unordered_map<ChunkCoord, std::unique_ptr<ChunkMesh>, ChunkCoordHash>& getMeshes() const;

private:
    void generateChunk(const ChunkCoord& coord);
    void meshChunk(const ChunkCoord& coord);

    bool shouldLoadChunk(const ChunkCoord& coord, const ChunkCoord& cameraCoord, int renderDistance) const;
    std::vector<ChunkCoord> getChunksInRadius(const ChunkCoord& center, int radius) const;
    void loadStreamingSettings();
    void enqueueGeneration(const ChunkCoord& coord);
    void enqueueMesh(const ChunkCoord& coord);
    void markNeighborChunksDirty(const ChunkCoord& coord);

    TextureAtlas* atlas;

    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> chunks;
    std::unordered_map<ChunkCoord, std::unique_ptr<ChunkMesh>, ChunkCoordHash> meshes;

    std::queue<ChunkCoord> generationQueue;
    std::queue<ChunkCoord> meshQueue;
    std::unordered_set<ChunkCoord, ChunkCoordHash> generationQueueSet;
    std::unordered_set<ChunkCoord, ChunkCoordHash> meshQueueSet;

    ChunkCoord lastCameraChunk;

    int chunksToGeneratePerFrame;
    int chunksToMeshPerFrame;
    int unloadMargin;
};

} // namespace PoorCraft

#include "poorcraft/world/ChunkManager.h"

#include <algorithm>
#include <array>

#include "poorcraft/core/Config.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BlockRegistry.h"

namespace PoorCraft {

namespace {

constexpr int DEFAULT_GENERATE_PER_FRAME = 1;
constexpr int DEFAULT_MESH_PER_FRAME = 2;
constexpr int DEFAULT_UNLOAD_MARGIN = 2;

} // namespace

ChunkManager::ChunkManager()
    : atlas(nullptr), chunks(), meshes(), generationQueue(), meshQueue(), generationQueueSet(), meshQueueSet(),
      lastCameraChunk(0, 0),
      chunksToGeneratePerFrame(DEFAULT_GENERATE_PER_FRAME), chunksToMeshPerFrame(DEFAULT_MESH_PER_FRAME),
      unloadMargin(DEFAULT_UNLOAD_MARGIN) {}

ChunkManager::~ChunkManager() {
    shutdown();
}

void ChunkManager::initialize() {
    PC_INFO("Initializing ChunkManager...");

    while (!generationQueue.empty()) {
        generationQueue.pop();
    }
    generationQueueSet.clear();

    while (!meshQueue.empty()) {
        meshQueue.pop();
    }
    meshQueueSet.clear();

    chunks.clear();
    meshes.clear();
    lastCameraChunk = ChunkCoord(0, 0);

    loadStreamingSettings();
}

void ChunkManager::shutdown() {
    PC_INFO("Shutting down ChunkManager. Removing " + std::to_string(chunks.size()) +
            " chunks and " + std::to_string(meshes.size()) + " meshes.");

    chunks.clear();
    meshes.clear();

    while (!generationQueue.empty()) {
        generationQueue.pop();
    }
    generationQueueSet.clear();

    while (!meshQueue.empty()) {
        meshQueue.pop();
    }
    meshQueueSet.clear();
}

void ChunkManager::update(const glm::vec3& cameraPosition, int renderDistance) {
    if (renderDistance < 0) {
        renderDistance = 0;
    }

    const ChunkCoord cameraChunk = ChunkCoord::fromWorldPos(cameraPosition.x, cameraPosition.z);

    if (cameraChunk != lastCameraChunk) {
        PC_DEBUG("Camera moved to chunk " + cameraChunk.toString());
        lastCameraChunk = cameraChunk;
    }

    const auto targetChunks = getChunksInRadius(cameraChunk, renderDistance);
    for (const auto& coord : targetChunks) {
        if (!hasChunk(coord)) {
            enqueueGeneration(coord);
        }
    }

    int generatedThisFrame = 0;
    while (!generationQueue.empty() && generatedThisFrame < chunksToGeneratePerFrame) {
        const ChunkCoord coord = generationQueue.front();
        generationQueue.pop();
        generationQueueSet.erase(coord);

        if (!hasChunk(coord)) {
            generateChunk(coord);
            ++generatedThisFrame;
        }
    }

    int meshedThisFrame = 0;
    while (!meshQueue.empty() && meshedThisFrame < chunksToMeshPerFrame) {
        const ChunkCoord coord = meshQueue.front();
        meshQueue.pop();
        meshQueueSet.erase(coord);
        meshChunk(coord);
        ++meshedThisFrame;
    }

    std::vector<ChunkCoord> toUnload;
    for (const auto& entry : chunks) {
        const ChunkCoord& coord = entry.first;
        const int distanceSquared = coord.getDistanceSquared(cameraChunk);
        const int maxDistance = renderDistance + unloadMargin;
        if (distanceSquared > maxDistance * maxDistance) {
            toUnload.push_back(coord);
        }
    }

    for (const auto& coord : toUnload) {
        unloadChunk(coord);
    }

    for (const auto& entry : chunks) {
        const ChunkCoord& coord = entry.first;
        Chunk& chunk = *entry.second;
        if (chunk.isDirty()) {
            enqueueMesh(coord);
            chunk.setDirty(false);
        }
    }
}

Chunk* ChunkManager::getChunk(const ChunkCoord& coord) const {
    const auto it = chunks.find(coord);
    if (it == chunks.end()) {
        return nullptr;
    }

    return it->second.get();
}

bool ChunkManager::hasChunk(const ChunkCoord& coord) const {
    return chunks.find(coord) != chunks.end();
}

Chunk& ChunkManager::getOrCreateChunk(const ChunkCoord& coord) {
    auto it = chunks.find(coord);
    if (it != chunks.end()) {
        return *it->second;
    }

    auto chunk = std::make_unique<Chunk>(coord);
    Chunk& reference = *chunk;
    chunks.emplace(coord, std::move(chunk));
    enqueueMesh(coord);
    markNeighborChunksDirty(coord);
    return reference;
}

void ChunkManager::unloadChunk(const ChunkCoord& coord) {
    PC_DEBUG("Unloading chunk " + coord.toString());
    chunks.erase(coord);
    meshes.erase(coord);
    markNeighborChunksDirty(coord);
}

std::size_t ChunkManager::getLoadedChunkCount() const {
    return chunks.size();
}

ChunkMesh* ChunkManager::getChunkMesh(const ChunkCoord& coord) const {
    const auto it = meshes.find(coord);
    if (it == meshes.end()) {
        return nullptr;
    }

    return it->second.get();
}

void ChunkManager::setTextureAtlas(TextureAtlas* atlasPtr) {
    atlas = atlasPtr;
}

const std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash>& ChunkManager::getChunks() const {
    return chunks;
}

const std::unordered_map<ChunkCoord, std::unique_ptr<ChunkMesh>, ChunkCoordHash>& ChunkManager::getMeshes() const {
    return meshes;
}

void ChunkManager::generateChunk(const ChunkCoord& coord) {
    auto chunk = std::make_unique<Chunk>(coord);

    auto& registry = BlockRegistry::getInstance();
    const uint16_t stoneId = registry.getBlockID("stone");
    const uint16_t dirtId = registry.getBlockID("dirt");
    const uint16_t grassId = registry.getBlockID("grass");

    for (int y = 0; y < 64; ++y) {
        const uint16_t blockId = y < 60 ? stoneId : dirtId;
        for (int z = 0; z < Chunk::CHUNK_SIZE_Z; ++z) {
            for (int x = 0; x < Chunk::CHUNK_SIZE_X; ++x) {
                chunk->setBlock(x, y, z, blockId);
            }
        }
    }

    for (int z = 0; z < Chunk::CHUNK_SIZE_Z; ++z) {
        for (int x = 0; x < Chunk::CHUNK_SIZE_X; ++x) {
            chunk->setBlock(x, 64, z, grassId);
        }
    }

    chunks.emplace(coord, std::move(chunk));
    PC_DEBUG("Generated chunk " + coord.toString());
    enqueueMesh(coord);
    markNeighborChunksDirty(coord);
}

void ChunkManager::meshChunk(const ChunkCoord& coord) {
    if (!atlas) {
        PC_WARN("Cannot mesh chunk " + coord.toString() + " without a texture atlas.");
        return;
    }

    auto chunkIt = chunks.find(coord);
    if (chunkIt == chunks.end()) {
        return;
    }

    Chunk& chunk = *chunkIt->second;
    if (chunk.isEmpty()) {
        meshes.erase(coord);
        return;
    }

    auto mesh = std::make_unique<ChunkMesh>(coord);
    if (mesh->generate(chunk, *this, *atlas)) {
        meshes[coord] = std::move(mesh);
        chunk.setDirty(false);
        PC_DEBUG("Meshed chunk " + coord.toString());
    } else {
        PC_WARN("Failed to mesh chunk " + coord.toString());
    }
}

bool ChunkManager::shouldLoadChunk(const ChunkCoord& coord,
                                   const ChunkCoord& cameraCoord,
                                   int renderDistance) const {
    const int distanceSquared = coord.getDistanceSquared(cameraCoord);
    return distanceSquared <= renderDistance * renderDistance;
}

std::vector<ChunkCoord> ChunkManager::getChunksInRadius(const ChunkCoord& center, int radius) const {
    std::vector<ChunkCoord> result;
    for (int x = center.x - radius; x <= center.x + radius; ++x) {
        for (int z = center.z - radius; z <= center.z + radius; ++z) {
            const ChunkCoord coord(x, z);
            if (shouldLoadChunk(coord, center, radius)) {
                result.push_back(coord);
            }
        }
    }
    return result;
}

void ChunkManager::loadStreamingSettings() {
    auto& config = poorcraft::Config::get_instance();
    chunksToGeneratePerFrame = config.get_int("World.chunk_generation_per_frame", DEFAULT_GENERATE_PER_FRAME);
    chunksToMeshPerFrame = config.get_int("World.chunk_meshing_per_frame", DEFAULT_MESH_PER_FRAME);
    unloadMargin = config.get_int("World.chunk_unload_margin", DEFAULT_UNLOAD_MARGIN);
}

void ChunkManager::enqueueGeneration(const ChunkCoord& coord) {
    if (generationQueueSet.insert(coord).second) {
        generationQueue.push(coord);
    }
}

void ChunkManager::enqueueMesh(const ChunkCoord& coord) {
    if (meshQueueSet.insert(coord).second) {
        meshQueue.push(coord);
    }
}

void ChunkManager::markNeighborChunksDirty(const ChunkCoord& coord) {
    static constexpr std::array<std::pair<int, int>, 4> NEIGHBOR_OFFSETS = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    for (const auto& offset : NEIGHBOR_OFFSETS) {
        ChunkCoord neighbor(coord.x + offset.first, coord.z + offset.second);
        auto it = chunks.find(neighbor);
        if (it != chunks.end()) {
            it->second->setDirty(true);
            enqueueMesh(neighbor);
        }
    }
}

} // namespace PoorCraft

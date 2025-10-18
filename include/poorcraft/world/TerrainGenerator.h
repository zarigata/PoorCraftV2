#pragma once

#include "poorcraft/world/BiomeMap.h"
#include "poorcraft/world/Chunk.h"
#include "poorcraft/world/ChunkCoord.h"

#include <FastNoiseLite.h>

#include <cstdint>

namespace PoorCraft {

class TerrainGenerator {
public:
    explicit TerrainGenerator(int64_t seed);
    TerrainGenerator(const TerrainGenerator&) = delete;
    TerrainGenerator& operator=(const TerrainGenerator&) = delete;
    TerrainGenerator(TerrainGenerator&&) = delete;
    TerrainGenerator& operator=(TerrainGenerator&&) = delete;
    ~TerrainGenerator() = default;

    void initialize();
    void setSeed(int64_t newSeed);

    void generateChunk(Chunk& chunk, const ChunkCoord& coord);

    [[nodiscard]] int getHeightAt(int32_t worldX, int32_t worldZ, BiomeType biome) const;
    [[nodiscard]] int getBlendedHeight(int32_t worldX, int32_t worldZ) const;

private:
    void generateTerrain(Chunk& chunk, const ChunkCoord& coord);
    void generateCaves(Chunk& chunk, const ChunkCoord& coord);
    void generateOres(Chunk& chunk, const ChunkCoord& coord);
    void placeStructures(Chunk& chunk, const ChunkCoord& coord);

    int64_t seed;
    BiomeMap biomeMap;
    FastNoiseLite terrainNoise;
    FastNoiseLite detailNoise;
    FastNoiseLite caveNoise;
    FastNoiseLite oreNoise;

    float caveDensity;
    float oreFrequency;
    float treeDensity;
};

} // namespace PoorCraft

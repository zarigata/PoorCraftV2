#include "poorcraft/world/TerrainGenerator.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BiomeType.h"
#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/StructureGenerator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace PoorCraft {
namespace {
struct OreDefinition {
    std::string name;
    int minY;
    int maxY;
    float threshold;
    uint16_t blockId;
};

[[nodiscard]] float normalizeHeightNoise(float value) {
    return (value + 1.0f) * 0.5f;
}

[[nodiscard]] uint16_t resolveBlock(uint16_t id, uint16_t fallback) {
    return id != 0 ? id : fallback;
}

[[nodiscard]] int clampToChunkHeight(int value) {
    return std::clamp(value, 1, Chunk::CHUNK_SIZE_Y - 1);
}

[[nodiscard]] float getCaveThreshold(float density) {
    const float baseThreshold = -0.3f;
    const float adjustment = (0.5f - density) * 0.2f;
    return baseThreshold + adjustment;
}

[[nodiscard]] uint32_t hashCoordinates(int64_t seed, int32_t x, int32_t z) {
    const int64_t hash = seed ^ (static_cast<int64_t>(x) * 0x9E3779B97F4A7C15ULL) ^
                         (static_cast<int64_t>(z) * 0xBF58476D1CE4E5B9ULL);
    return static_cast<uint32_t>((hash >> 32) ^ hash);
}

} // namespace

TerrainGenerator::TerrainGenerator(int64_t seedValue)
    : seed(seedValue), biomeMap(seedValue), terrainNoise(), detailNoise(), caveNoise(), oreNoise(),
      caveDensity(0.5f), oreFrequency(1.0f), treeDensity(1.0f) {}

void TerrainGenerator::initialize() {
    auto& config = poorcraft::Config::get_instance();

    caveDensity = std::clamp(config.get_float("World.cave_density", 0.5f), 0.0f, 1.0f);
    oreFrequency = std::max(0.0f, config.get_float("World.ore_frequency", 1.0f));
    treeDensity = std::max(0.0f, config.get_float("World.tree_density", 1.0f));
    const float biomeScale = std::max(0.1f, config.get_float("World.biome_scale", 1.0f));
    biomeMap.setBiomeScale(biomeScale);

    terrainNoise.SetSeed(static_cast<int32_t>(seed));
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    terrainNoise.SetFrequency(0.0015f);
    terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    terrainNoise.SetFractalOctaves(6);
    terrainNoise.SetFractalGain(0.5f);

    detailNoise.SetSeed(static_cast<int32_t>(seed + 100));
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    detailNoise.SetFrequency(0.01f);
    detailNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    detailNoise.SetFractalOctaves(3);

    caveNoise.SetSeed(static_cast<int32_t>(seed + 200));
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    caveNoise.SetFrequency(0.02f);
    caveNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    caveNoise.SetFractalOctaves(2);

    oreNoise.SetSeed(static_cast<int32_t>(seed + 300));
    oreNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    oreNoise.SetFrequency(0.05f);

    PC_INFO("TerrainGenerator initialized with seed " + std::to_string(seed));
}

void TerrainGenerator::setSeed(int64_t newSeed) {
    seed = newSeed;
    biomeMap = BiomeMap(seed);
    initialize();
}

void TerrainGenerator::generateChunk(Chunk& chunk, const ChunkCoord& coord) {
    generateTerrain(chunk, coord);
    generateCaves(chunk, coord);
    generateOres(chunk, coord);
    placeStructures(chunk, coord);

    PC_DEBUG("Generated terrain for chunk " + coord.toString());
}

int TerrainGenerator::getHeightAt(int32_t worldX, int32_t worldZ, BiomeType biome) const {
    const auto& definition = getBiomeDefinition(biome);

    const float baseNoise = normalizeHeightNoise(terrainNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ)));
    const float detail = detailNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ)) * 2.0f;

    float height = static_cast<float>(definition.baseHeight);
    height += baseNoise * static_cast<float>(definition.heightVariation);
    height += detail;

    return clampToChunkHeight(static_cast<int>(std::round(height)));
}

int TerrainGenerator::getBlendedHeight(int32_t worldX, int32_t worldZ) const {
    const auto blends = biomeMap.getBlendedBiomes(worldX, worldZ);

    float accumulatedHeight = 0.0f;
    float totalWeight = 0.0f;

    for (const auto& [biome, weight] : blends) {
        accumulatedHeight += static_cast<float>(getHeightAt(worldX, worldZ, biome)) * weight;
        totalWeight += weight;
    }

    if (totalWeight <= 0.0f) {
        return getHeightAt(worldX, worldZ, biomeMap.getBiomeAt(worldX, worldZ));
    }

    return clampToChunkHeight(static_cast<int>(std::round(accumulatedHeight / totalWeight)));
}

void TerrainGenerator::generateTerrain(Chunk& chunk, const ChunkCoord& coord) {
    auto& registry = BlockRegistry::getInstance();

    const uint16_t defaultStone = registry.getBlockID("stone");
    const uint16_t bedrockId = resolveBlock(registry.getBlockID("bedrock"), defaultStone);

    for (int localZ = 0; localZ < Chunk::CHUNK_SIZE_Z; ++localZ) {
        for (int localX = 0; localX < Chunk::CHUNK_SIZE_X; ++localX) {
            const int32_t worldX = coord.x * Chunk::CHUNK_SIZE_X + localX;
            const int32_t worldZ = coord.z * Chunk::CHUNK_SIZE_Z + localZ;

            const int height = getBlendedHeight(worldX, worldZ);
            const BiomeType biome = biomeMap.getBiomeAt(worldX, worldZ);
            const auto& definition = getBiomeDefinition(biome);

            const uint16_t surfaceId = resolveBlock(definition.surfaceBlock, defaultStone);
            const uint16_t subsurfaceId = resolveBlock(definition.subsurfaceBlock, defaultStone);
            const uint16_t undergroundId = resolveBlock(definition.undergroundBlock, defaultStone);

            for (int localY = 0; localY <= height; ++localY) {
                if (localY == 0) {
                    chunk.setBlock(localX, localY, localZ, bedrockId);
                    continue;
                }

                if (localY < height - 4) {
                    chunk.setBlock(localX, localY, localZ, undergroundId);
                } else if (localY < height - 1) {
                    chunk.setBlock(localX, localY, localZ, subsurfaceId);
                } else {
                    chunk.setBlock(localX, localY, localZ, surfaceId);
                }
            }
        }
    }
}

void TerrainGenerator::generateCaves(Chunk& chunk, const ChunkCoord& coord) {
    auto& registry = BlockRegistry::getInstance();

    const uint16_t bedrockId = registry.getBlockID("bedrock");
    const uint16_t lavaId = registry.getBlockID("lava");

    const float threshold = getCaveThreshold(caveDensity);

    for (int localZ = 0; localZ < Chunk::CHUNK_SIZE_Z; ++localZ) {
        for (int localX = 0; localX < Chunk::CHUNK_SIZE_X; ++localX) {
            const int32_t worldX = coord.x * Chunk::CHUNK_SIZE_X + localX;
            const int32_t worldZ = coord.z * Chunk::CHUNK_SIZE_Z + localZ;

            for (int localY = 1; localY < Chunk::CHUNK_SIZE_Y - 1; ++localY) {
                const uint16_t currentBlock = chunk.getBlock(localX, localY, localZ);
                if (currentBlock == 0 || currentBlock == bedrockId) {
                    continue;
                }

                const float noise = caveNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(localY), static_cast<float>(worldZ));
                if (noise < threshold) {
                    if (localY < 10 && lavaId != 0) {
                        chunk.setBlock(localX, localY, localZ, lavaId);
                    } else {
                        chunk.setBlock(localX, localY, localZ, 0);
                    }
                }
            }
        }
    }
}

void TerrainGenerator::generateOres(Chunk& chunk, const ChunkCoord& coord) {
    auto& registry = BlockRegistry::getInstance();

    const uint16_t stoneId = registry.getBlockID("stone");
    const uint16_t sandstoneId = registry.getBlockID("sandstone");

    std::array<OreDefinition, 4> ores = {
        OreDefinition{"coal_ore", 5, 128, 0.4f, registry.getBlockID("coal_ore")},
        OreDefinition{"iron_ore", 5, 64, 0.55f, registry.getBlockID("iron_ore")},
        OreDefinition{"gold_ore", 5, 32, 0.65f, registry.getBlockID("gold_ore")},
        OreDefinition{"diamond_ore", 5, 16, 0.75f, registry.getBlockID("diamond_ore")}
    };

    for (int localZ = 0; localZ < Chunk::CHUNK_SIZE_Z; ++localZ) {
        for (int localX = 0; localX < Chunk::CHUNK_SIZE_X; ++localX) {
            const int32_t worldX = coord.x * Chunk::CHUNK_SIZE_X + localX;
            const int32_t worldZ = coord.z * Chunk::CHUNK_SIZE_Z + localZ;

            for (auto& ore : ores) {
                if (ore.blockId == 0) {
                    continue;
                }

                const float adjustedThreshold = std::clamp(ore.threshold - (oreFrequency - 1.0f) * 0.1f, -1.0f, 0.95f);

                for (int localY = ore.minY; localY <= ore.maxY && localY < Chunk::CHUNK_SIZE_Y; ++localY) {
                    const float noise = oreNoise.GetNoise(static_cast<float>(worldX), static_cast<float>(localY), static_cast<float>(worldZ));
                    if (noise <= adjustedThreshold) {
                        continue;
                    }

                    for (int dx = 0; dx < 2; ++dx) {
                        for (int dy = 0; dy < 2; ++dy) {
                            for (int dz = 0; dz < 2; ++dz) {
                                const int nx = localX + dx;
                                const int ny = localY + dy;
                                const int nz = localZ + dz;

                                if (nx >= Chunk::CHUNK_SIZE_X || ny >= Chunk::CHUNK_SIZE_Y || nz >= Chunk::CHUNK_SIZE_Z) {
                                    continue;
                                }

                                const uint16_t current = chunk.getBlock(nx, ny, nz);
                                if (current == stoneId || current == sandstoneId) {
                                    chunk.setBlock(nx, ny, nz, ore.blockId);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void TerrainGenerator::placeStructures(Chunk& chunk, const ChunkCoord& coord) {
    StructureGenerator structureGenerator(seed);

    std::minstd_rand rng(hashCoordinates(seed, coord.x, coord.z));
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    for (int localZ = 0; localZ < Chunk::CHUNK_SIZE_Z; ++localZ) {
        for (int localX = 0; localX < Chunk::CHUNK_SIZE_X; ++localX) {
            int topY = -1;
            for (int localY = Chunk::CHUNK_SIZE_Y - 1; localY >= 0; --localY) {
                if (chunk.getBlock(localX, localY, localZ) != 0) {
                    topY = localY;
                    break;
                }
            }

            if (topY <= 0) {
                continue;
            }

            const int32_t worldX = coord.x * Chunk::CHUNK_SIZE_X + localX;
            const int32_t worldZ = coord.z * Chunk::CHUNK_SIZE_Z + localZ;

            const BiomeType biome = biomeMap.getBiomeAt(worldX, worldZ);
            const auto& definition = getBiomeDefinition(biome);

            const float treeChance = std::clamp(definition.treeChance * treeDensity, 0.0f, 1.0f);
            if (treeChance > 0.0f && distribution(rng) < treeChance) {
                structureGenerator.placeTree(chunk, localX, topY, localZ, biome);
                continue;
            }

            const float decorationChance = std::clamp(definition.grassChance * treeDensity, 0.0f, 1.0f);
            if (decorationChance <= 0.0f || distribution(rng) >= decorationChance) {
                continue;
            }

            for (const auto& feature : definition.specialFeatures) {
                switch (feature) {
                case BiomeFeature::CACTUS:
                    structureGenerator.placeCactus(chunk, localX, topY, localZ);
                    break;
                case BiomeFeature::TALL_GRASS:
                    structureGenerator.placeTallGrass(chunk, localX, topY, localZ);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

} // namespace PoorCraft

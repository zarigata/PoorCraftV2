#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace PoorCraft {

enum class BiomeType {
    PLAINS = 0,
    DESERT,
    SNOW,
    JUNGLE,
    MOUNTAINS
};

enum class BiomeFeature {
    NONE = 0,
    CACTUS,
    TALL_GRASS,
    FLOWERS,
    VINES
};

struct RangeF {
    float min;
    float max;
};

struct BiomeDefinition {
    BiomeType type;
    std::string name;
    int baseHeight;
    int heightVariation;
    uint16_t surfaceBlock;
    uint16_t subsurfaceBlock;
    uint16_t undergroundBlock;
    RangeF temperatureRange;
    RangeF humidityRange;
    float treeChance;
    float grassChance;
    std::vector<BiomeFeature> specialFeatures;
};

const BiomeDefinition& getBiomeDefinition(BiomeType biome);
std::string getBiomeName(BiomeType biome);

} // namespace PoorCraft

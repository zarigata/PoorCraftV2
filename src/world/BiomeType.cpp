#include "poorcraft/world/BiomeType.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/world/BlockRegistry.h"

#include <array>
#include <mutex>

namespace PoorCraft {

namespace {
std::mutex& getBiomeMutex() {
    static std::mutex mutex;
    return mutex;
}

std::array<BiomeDefinition, 5>& getBiomeDefinitionsInternal() {
    static std::array<BiomeDefinition, 5> definitions{};
    return definitions;
}

bool& getBiomeInitializedFlag() {
    static bool initialized = false;
    return initialized;
}

void initializeBiomes() {
    std::scoped_lock lock(getBiomeMutex());

    if (getBiomeInitializedFlag()) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();

    auto grassId = registry.getBlockID("grass");
    auto dirtId = registry.getBlockID("dirt");
    auto stoneId = registry.getBlockID("stone");
    auto sandId = registry.getBlockID("sand");
    auto sandstoneId = registry.getBlockID("sandstone");
    auto snowId = registry.getBlockID("snow");
    auto snowGrassId = registry.getBlockID("snow_grass");
    auto iceId = registry.getBlockID("ice");

    auto& defs = getBiomeDefinitionsInternal();

    defs[static_cast<size_t>(BiomeType::PLAINS)] = {
        BiomeType::PLAINS,
        "Plains",
        64,
        8,
        grassId,
        dirtId,
        stoneId,
        {0.5f, 0.8f},
        {0.4f, 0.7f},
        0.05f,
        0.6f,
        {BiomeFeature::FLOWERS, BiomeFeature::TALL_GRASS}
    };

    defs[static_cast<size_t>(BiomeType::DESERT)] = {
        BiomeType::DESERT,
        "Desert",
        64,
        4,
        sandId,
        sandId,
        sandstoneId,
        {0.8f, 1.0f},
        {0.0f, 0.2f},
        0.0f,
        0.05f,
        {BiomeFeature::CACTUS}
    };

    defs[static_cast<size_t>(BiomeType::SNOW)] = {
        BiomeType::SNOW,
        "Snow",
        64,
        6,
        snowGrassId,
        dirtId,
        stoneId,
        {0.0f, 0.3f},
        {0.3f, 0.6f},
        0.02f,
        0.2f,
        {BiomeFeature::TALL_GRASS}
    };

    defs[static_cast<size_t>(BiomeType::JUNGLE)] = {
        BiomeType::JUNGLE,
        "Jungle",
        64,
        10,
        grassId,
        dirtId,
        stoneId,
        {0.8f, 1.0f},
        {0.7f, 1.0f},
        0.15f,
        0.8f,
        {BiomeFeature::TALL_GRASS, BiomeFeature::VINES}
    };

    defs[static_cast<size_t>(BiomeType::MOUNTAINS)] = {
        BiomeType::MOUNTAINS,
        "Mountains",
        80,
        40,
        stoneId,
        stoneId,
        stoneId,
        {0.2f, 0.6f},
        {0.3f, 0.7f},
        0.01f,
        0.1f,
        {BiomeFeature::FLOWERS}
    };

    getBiomeInitializedFlag() = true;
}

} // namespace

const BiomeDefinition& getBiomeDefinition(BiomeType biome) {
    if (!getBiomeInitializedFlag()) {
        initializeBiomes();
    }

    auto index = static_cast<size_t>(biome);
    auto& defs = getBiomeDefinitionsInternal();

    if (index >= defs.size()) {
        PC_WARN("Requested invalid biome definition index: " + std::to_string(index));
        return defs[0];
    }

    return defs[index];
}

std::string getBiomeName(BiomeType biome) {
    switch (biome) {
    case BiomeType::PLAINS:
        return "Plains";
    case BiomeType::DESERT:
        return "Desert";
    case BiomeType::SNOW:
        return "Snow";
    case BiomeType::JUNGLE:
        return "Jungle";
    case BiomeType::MOUNTAINS:
        return "Mountains";
    default:
        PC_WARN("Requested name for unknown biome type");
        return "Unknown";
    }
}

} // namespace PoorCraft

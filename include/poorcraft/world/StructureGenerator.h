#pragma once

#include "poorcraft/world/BiomeType.h"
#include "poorcraft/world/Chunk.h"

#include <cstdint>

namespace PoorCraft {

enum class StructureType {
    OAK_TREE,
    JUNGLE_TREE,
    SPRUCE_TREE,
    CACTUS,
    TALL_GRASS,
    FLOWER
};

class StructureGenerator {
public:
    explicit StructureGenerator(int64_t seed);
    StructureGenerator(const StructureGenerator&) = delete;
    StructureGenerator& operator=(const StructureGenerator&) = delete;
    StructureGenerator(StructureGenerator&&) noexcept = default;
    StructureGenerator& operator=(StructureGenerator&&) noexcept = default;
    ~StructureGenerator() = default;

    void placeTree(Chunk& chunk, int x, int y, int z, BiomeType biome);
    void placeCactus(Chunk& chunk, int x, int y, int z);
    void placeTallGrass(Chunk& chunk, int x, int y, int z);
    void placeFlower(Chunk& chunk, int x, int y, int z);

private:
    [[nodiscard]] bool canPlaceStructure(const Chunk& chunk, int x, int y, int z, StructureType type) const;

    void placeOakTree(Chunk& chunk, int x, int y, int z);
    void placeJungleTree(Chunk& chunk, int x, int y, int z);
    void placeSpruceTree(Chunk& chunk, int x, int y, int z);

    [[nodiscard]] int getRandomInRange(int minValue, int maxValue, int x, int y, int z) const;

    int64_t seed;
};

} // namespace PoorCraft

#include "poorcraft/world/StructureGenerator.h"

#include "poorcraft/world/BlockRegistry.h"
#include "poorcraft/world/Chunk.h"

#include <algorithm>
#include <array>

namespace PoorCraft {
namespace {
constexpr int CHUNK_MIN_Y = 0;
constexpr int CHUNK_MAX_Y = Chunk::CHUNK_SIZE_Y - 1;

[[nodiscard]] bool isWithinChunk(int x, int y, int z) {
    return x >= 0 && x < Chunk::CHUNK_SIZE_X && y >= CHUNK_MIN_Y && y <= CHUNK_MAX_Y && z >= 0 && z < Chunk::CHUNK_SIZE_Z;
}

[[nodiscard]] bool hasAirAround(const Chunk& chunk, int x, int y, int z) {
    static constexpr std::array<std::pair<int, int>, 4> OFFSETS = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    for (const auto& [dx, dz] : OFFSETS) {
        const int nx = x + dx;
        const int nz = z + dz;
        if (!isWithinChunk(nx, y, nz)) {
            return false;
        }
        if (chunk.getBlock(nx, y, nz) != 0) {
            return false;
        }
    }
    return true;
}

} // namespace

StructureGenerator::StructureGenerator(int64_t worldSeed)
    : seed(worldSeed) {}

void StructureGenerator::placeTree(Chunk& chunk, int x, int y, int z, BiomeType biome) {
    switch (biome) {
    case BiomeType::PLAINS:
    case BiomeType::MOUNTAINS:
        placeOakTree(chunk, x, y, z);
        break;
    case BiomeType::JUNGLE:
        placeJungleTree(chunk, x, y, z);
        break;
    case BiomeType::SNOW:
        placeSpruceTree(chunk, x, y, z);
        break;
    case BiomeType::DESERT:
        return;
    }
}

void StructureGenerator::placeCactus(Chunk& chunk, int x, int y, int z) {
    if (!isWithinChunk(x, y, z) || !isWithinChunk(x, y + 3, z)) {
        return;
    }

    if (!canPlaceStructure(chunk, x, y, z, StructureType::CACTUS)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t cactusId = registry.getBlockID("cactus");

    const int height = getRandomInRange(1, 3, x, y, z);
    for (int h = 0; h < height; ++h) {
        const int ny = y + h;
        if (!isWithinChunk(x, ny, z)) {
            break;
        }
        if (!hasAirAround(chunk, x, ny, z)) {
            break;
        }
        chunk.setBlock(x, ny, z, cactusId);
    }
}

void StructureGenerator::placeTallGrass(Chunk& chunk, int x, int y, int z) {
    if (!isWithinChunk(x, y, z) || !isWithinChunk(x, y + 1, z)) {
        return;
    }

    if (!canPlaceStructure(chunk, x, y, z, StructureType::TALL_GRASS)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t tallGrassId = registry.getBlockID("tall_grass");

    chunk.setBlock(x, y + 1, z, tallGrassId);
}

void StructureGenerator::placeFlower(Chunk& chunk, int x, int y, int z) {
    if (!isWithinChunk(x, y, z) || !isWithinChunk(x, y + 1, z)) {
        return;
    }

    if (!canPlaceStructure(chunk, x, y, z, StructureType::FLOWER)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t flowerId = registry.getBlockID("flower");

    if (flowerId == 0) {
        return;
    }

    chunk.setBlock(x, y + 1, z, flowerId);
}

bool StructureGenerator::canPlaceStructure(const Chunk& chunk, int x, int y, int z, StructureType type) const {
    auto& registry = BlockRegistry::getInstance();
    const uint16_t groundBlock = chunk.getBlock(x, y, z);

    switch (type) {
    case StructureType::OAK_TREE:
    case StructureType::JUNGLE_TREE:
    case StructureType::SPRUCE_TREE: {
        if (groundBlock == 0) {
            return false;
        }
        const std::array<std::string, 3> validGround = {"grass", "dirt", "snow_grass"};
        bool valid = false;
        for (const auto& name : validGround) {
            if (registry.getBlockID(name) == groundBlock) {
                valid = true;
                break;
            }
        }
        if (!valid) {
            return false;
        }
        const int minY = y + 1;
        const int maxY = y + 12;
        for (int ny = minY; ny <= maxY && ny <= CHUNK_MAX_Y; ++ny) {
            for (int nx = x - 3; nx <= x + 3; ++nx) {
                for (int nz = z - 3; nz <= z + 3; ++nz) {
                    if (!isWithinChunk(nx, ny, nz)) {
                        continue;
                    }
                    if (chunk.getBlock(nx, ny, nz) != 0) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    case StructureType::CACTUS: {
        const uint16_t sandId = registry.getBlockID("sand");
        if (groundBlock != sandId) {
            return false;
        }
        return true;
    }
    case StructureType::TALL_GRASS:
    case StructureType::FLOWER: {
        const uint16_t grassId = registry.getBlockID("grass");
        const uint16_t snowGrassId = registry.getBlockID("snow_grass");
        const uint16_t dirtId = registry.getBlockID("dirt");
        bool validGround = groundBlock == grassId || groundBlock == snowGrassId || groundBlock == dirtId;
        if (!validGround) {
            return false;
        }
        if (chunk.getBlock(x, y + 1, z) != 0) {
            return false;
        }
        return true;
    }
    }

    return false;
}

void StructureGenerator::placeOakTree(Chunk& chunk, int x, int y, int z) {
    if (!canPlaceStructure(chunk, x, y, z, StructureType::OAK_TREE)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t logId = registry.getBlockID("oak_log");
    const uint16_t leavesId = registry.getBlockID("oak_leaves");

    const int height = getRandomInRange(4, 6, x, y, z);

    for (int h = 1; h <= height; ++h) {
        const int ny = y + h;
        if (!isWithinChunk(x, ny, z)) {
            break;
        }
        chunk.setBlock(x, ny, z, logId);
    }

    const int canopyBase = y + height - 1;
    const int canopyTop = y + height + 1;

    for (int ny = canopyBase; ny <= canopyTop; ++ny) {
        const int layer = ny - canopyBase;
        const int radius = layer == 0 ? 2 : (layer == canopyTop - canopyBase ? 1 : 2);
        for (int nx = x - radius; nx <= x + radius; ++nx) {
            for (int nz = z - radius; nz <= z + radius; ++nz) {
                if (!isWithinChunk(nx, ny, nz)) {
                    continue;
                }
                if (chunk.getBlock(nx, ny, nz) != 0) {
                    continue;
                }
                const int dx = nx - x;
                const int dz = nz - z;
                const float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));
                if (distance <= static_cast<float>(radius) + 0.2f) {
                    chunk.setBlock(nx, ny, nz, leavesId);
                }
            }
        }
    }
}

void StructureGenerator::placeJungleTree(Chunk& chunk, int x, int y, int z) {
    if (!canPlaceStructure(chunk, x, y, z, StructureType::JUNGLE_TREE)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t logId = registry.getBlockID("jungle_log");
    const uint16_t leavesId = registry.getBlockID("jungle_leaves");
    const uint16_t vinesId = registry.getBlockID("vines");

    const int height = getRandomInRange(8, 12, x, y, z);

    for (int h = 1; h <= height; ++h) {
        const int ny = y + h;
        if (!isWithinChunk(x, ny, z)) {
            break;
        }
        chunk.setBlock(x, ny, z, logId);
    }

    const int canopyBase = y + height - 2;
    const int canopyTop = y + height + 2;

    for (int ny = canopyBase; ny <= canopyTop; ++ny) {
        const int layerOffset = ny - canopyBase;
        const int radius = 2 + layerOffset / 2;
        for (int nx = x - radius; nx <= x + radius; ++nx) {
            for (int nz = z - radius; nz <= z + radius; ++nz) {
                if (!isWithinChunk(nx, ny, nz)) {
                    continue;
                }
                if (chunk.getBlock(nx, ny, nz) != 0) {
                    continue;
                }
                const int dx = nx - x;
                const int dz = nz - z;
                const float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));
                if (distance <= static_cast<float>(radius) + 0.5f) {
                    chunk.setBlock(nx, ny, nz, leavesId);
                    if (getRandomInRange(0, 3, nx, ny, nz) == 0) {
                        for (int v = 1; v <= 3; ++v) {
                            const int vineY = ny - v;
                            if (!isWithinChunk(nx, vineY, nz)) {
                                break;
                            }
                            if (chunk.getBlock(nx, vineY, nz) != 0) {
                                break;
                            }
                            chunk.setBlock(nx, vineY, nz, vinesId);
                        }
                    }
                }
            }
        }
    }
}

void StructureGenerator::placeSpruceTree(Chunk& chunk, int x, int y, int z) {
    if (!canPlaceStructure(chunk, x, y, z, StructureType::SPRUCE_TREE)) {
        return;
    }

    auto& registry = BlockRegistry::getInstance();
    const uint16_t logId = registry.getBlockID("spruce_log");
    const uint16_t leavesId = registry.getBlockID("spruce_leaves");

    const int height = getRandomInRange(6, 10, x, y, z);

    for (int h = 1; h <= height; ++h) {
        const int ny = y + h;
        if (!isWithinChunk(x, ny, z)) {
            break;
        }
        chunk.setBlock(x, ny, z, logId);
    }

    const int canopyBase = y + height / 2;
    const int canopyTop = y + height;

    for (int ny = canopyBase; ny <= canopyTop; ++ny) {
        const int layer = ny - canopyBase;
        const int radius = std::max(1, 3 - layer / 2);
        for (int nx = x - radius; nx <= x + radius; ++nx) {
            for (int nz = z - radius; nz <= z + radius; ++nz) {
                if (!isWithinChunk(nx, ny, nz)) {
                    continue;
                }
                if (chunk.getBlock(nx, ny, nz) != 0) {
                    continue;
                }
                const int dx = nx - x;
                const int dz = nz - z;
                const float distance = std::sqrt(static_cast<float>(dx * dx + dz * dz));
                if (distance <= static_cast<float>(radius) + 0.3f) {
                    chunk.setBlock(nx, ny, nz, leavesId);
                }
            }
        }
    }
}

int StructureGenerator::getRandomInRange(int minValue, int maxValue, int x, int y, int z) const {
    if (minValue >= maxValue) {
        return minValue;
    }

    const int64_t hash = seed ^ (static_cast<int64_t>(x) * 73856093LL) ^
                         (static_cast<int64_t>(y) * 19349663LL) ^
                         (static_cast<int64_t>(z) * 83492791LL);
    int64_t value = ((hash >> 16) ^ hash) & 0x7FFFFFFFLL;
    const int range = maxValue - minValue + 1;
    value = value % range;
    return minValue + static_cast<int>(value);
}

} // namespace PoorCraft

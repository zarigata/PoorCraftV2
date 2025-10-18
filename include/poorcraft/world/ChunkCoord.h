#pragma once

#include <cstdint>
#include <string>

#include <glm/vec3.hpp>

namespace PoorCraft {

struct ChunkCoord {
    int32_t x;
    int32_t z;

    ChunkCoord() : x(0), z(0) {}
    ChunkCoord(int32_t chunkX, int32_t chunkZ) : x(chunkX), z(chunkZ) {}

    bool operator==(const ChunkCoord& other) const;
    bool operator!=(const ChunkCoord& other) const;
    bool operator<(const ChunkCoord& other) const;

    static ChunkCoord fromWorldPos(float worldX, float worldZ);

    [[nodiscard]] glm::vec3 toWorldPos() const;
    [[nodiscard]] int32_t getDistance(const ChunkCoord& other) const;
    [[nodiscard]] int32_t getDistanceSquared(const ChunkCoord& other) const;
    [[nodiscard]] std::string toString() const;
};

struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& coord) const noexcept;
};

} // namespace PoorCraft

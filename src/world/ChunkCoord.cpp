#include "poorcraft/world/ChunkCoord.h"

#include <cmath>

namespace PoorCraft {

bool ChunkCoord::operator==(const ChunkCoord& other) const {
    return x == other.x && z == other.z;
}

bool ChunkCoord::operator!=(const ChunkCoord& other) const {
    return !(*this == other);
}

bool ChunkCoord::operator<(const ChunkCoord& other) const {
    return x < other.x || (x == other.x && z < other.z);
}

ChunkCoord ChunkCoord::fromWorldPos(float worldX, float worldZ) {
    const auto chunkX = static_cast<int32_t>(std::floor(worldX / 16.0f));
    const auto chunkZ = static_cast<int32_t>(std::floor(worldZ / 16.0f));
    return {chunkX, chunkZ};
}

glm::vec3 ChunkCoord::toWorldPos() const {
    return {static_cast<float>(x) * 16.0f, 0.0f, static_cast<float>(z) * 16.0f};
}

int32_t ChunkCoord::getDistance(const ChunkCoord& other) const {
    return std::abs(x - other.x) + std::abs(z - other.z);
}

int32_t ChunkCoord::getDistanceSquared(const ChunkCoord& other) const {
    const auto dx = x - other.x;
    const auto dz = z - other.z;
    return dx * dx + dz * dz;
}

std::string ChunkCoord::toString() const {
    return "Chunk(" + std::to_string(x) + ", " + std::to_string(z) + ")";
}

size_t ChunkCoordHash::operator()(const ChunkCoord& coord) const noexcept {
    static std::hash<int32_t> hasher;
    return hasher(coord.x) ^ (hasher(coord.z) << 1);
}

} // namespace PoorCraft

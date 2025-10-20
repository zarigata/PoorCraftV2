#pragma once

#include "poorcraft/world/ChunkCoord.h"

#include <array>
#include <cstdint>

namespace PoorCraft {

class Chunk {
public:
    static constexpr int32_t CHUNK_SIZE_X = 16;
    static constexpr int32_t CHUNK_SIZE_Y = 256;
    static constexpr int32_t CHUNK_SIZE_Z = 16;
    static constexpr int32_t CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

    explicit Chunk(const ChunkCoord& chunkPosition);
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) noexcept = default;
    Chunk& operator=(Chunk&&) noexcept = default;

    [[nodiscard]] uint16_t getBlock(int32_t x, int32_t y, int32_t z) const;
    void setBlock(int32_t x, int32_t y, int32_t z, uint16_t blockId);

    bool getBlockSafe(int32_t x, int32_t y, int32_t z, uint16_t& outBlock) const;
    void fill(uint16_t blockId);

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] const ChunkCoord& getPosition() const;

    [[nodiscard]] bool isDirty() const;
    void setDirty(bool dirtyState);

    [[nodiscard]] uint32_t getBlockCount() const;

    // Lighting data access (0-15 light levels)
    [[nodiscard]] uint8_t getSkyLight(int32_t x, int32_t y, int32_t z) const;
    [[nodiscard]] uint8_t getBlockLight(int32_t x, int32_t y, int32_t z) const;
    void setSkyLight(int32_t x, int32_t y, int32_t z, uint8_t level);
    void setBlockLight(int32_t x, int32_t y, int32_t z, uint8_t level);
    [[nodiscard]] uint8_t getLightLevel(int32_t x, int32_t y, int32_t z) const;
    void fillSkyLight(uint8_t level);
    void fillBlockLight(uint8_t level);

private:
    [[nodiscard]] static bool isValidPosition(int32_t x, int32_t y, int32_t z);
    [[nodiscard]] static int32_t getIndex(int32_t x, int32_t y, int32_t z);

    ChunkCoord position;
    std::array<uint16_t, CHUNK_VOLUME> blocks;
    std::array<uint8_t, CHUNK_VOLUME> skyLight;   // Sky light levels (0-15, sunlight from above)
    std::array<uint8_t, CHUNK_VOLUME> blockLight; // Block light levels (0-15, emissive blocks)
    bool dirty;
    uint32_t blockCount;
};

} // namespace PoorCraft

#include "poorcraft/world/Chunk.h"

#include <algorithm>

namespace PoorCraft {

Chunk::Chunk(const ChunkCoord& chunkPosition)
    : position(chunkPosition), blocks(), skyLight(), blockLight(), dirty(true), blockCount(0) {
    blocks.fill(0);
    skyLight.fill(15);  // Initialize with full sunlight
    blockLight.fill(0); // Initialize with no emissive light
}

uint16_t Chunk::getBlock(int32_t x, int32_t y, int32_t z) const {
    if (!isValidPosition(x, y, z)) {
        return 0;
    }

    return blocks[static_cast<size_t>(getIndex(x, y, z))];
}

void Chunk::setBlock(int32_t x, int32_t y, int32_t z, uint16_t blockId) {
    if (!isValidPosition(x, y, z)) {
        return;
    }

    const auto index = static_cast<size_t>(getIndex(x, y, z));
    const auto previous = blocks[index];

    if (previous == blockId) {
        return;
    }

    blocks[index] = blockId;

    if (previous != 0) {
        --blockCount;
    }

    if (blockId != 0) {
        ++blockCount;
    }

    dirty = true;
}

bool Chunk::getBlockSafe(int32_t x, int32_t y, int32_t z, uint16_t& outBlock) const {
    if (!isValidPosition(x, y, z)) {
        outBlock = 0;
        return false;
    }

    outBlock = blocks[static_cast<size_t>(getIndex(x, y, z))];
    return true;
}

void Chunk::fill(uint16_t blockId) {
    blocks.fill(blockId);
    blockCount = blockId == 0 ? 0 : static_cast<uint32_t>(CHUNK_VOLUME);
    dirty = true;
}

bool Chunk::isEmpty() const {
    return blockCount == 0;
}

const ChunkCoord& Chunk::getPosition() const {
    return position;
}

bool Chunk::isDirty() const {
    return dirty;
}

void Chunk::setDirty(bool dirtyState) {
    dirty = dirtyState;
}

uint32_t Chunk::getBlockCount() const {
    return blockCount;
}

bool Chunk::isValidPosition(int32_t x, int32_t y, int32_t z) {
    return x >= 0 && x < CHUNK_SIZE_X && y >= 0 && y < CHUNK_SIZE_Y && z >= 0 && z < CHUNK_SIZE_Z;
}

int32_t Chunk::getIndex(int32_t x, int32_t y, int32_t z) {
    return x + (z * CHUNK_SIZE_X) + (y * CHUNK_SIZE_X * CHUNK_SIZE_Z);
}

uint8_t Chunk::getSkyLight(int32_t x, int32_t y, int32_t z) const {
    if (!isValidPosition(x, y, z)) {
        return 0;
    }
    return skyLight[static_cast<size_t>(getIndex(x, y, z))];
}

uint8_t Chunk::getBlockLight(int32_t x, int32_t y, int32_t z) const {
    if (!isValidPosition(x, y, z)) {
        return 0;
    }
    return blockLight[static_cast<size_t>(getIndex(x, y, z))];
}

void Chunk::setSkyLight(int32_t x, int32_t y, int32_t z, uint8_t level) {
    if (!isValidPosition(x, y, z)) {
        return;
    }
    skyLight[static_cast<size_t>(getIndex(x, y, z))] = level;
    dirty = true;
}

void Chunk::setBlockLight(int32_t x, int32_t y, int32_t z, uint8_t level) {
    if (!isValidPosition(x, y, z)) {
        return;
    }
    blockLight[static_cast<size_t>(getIndex(x, y, z))] = level;
    dirty = true;
}

uint8_t Chunk::getLightLevel(int32_t x, int32_t y, int32_t z) const {
    return std::max(getSkyLight(x, y, z), getBlockLight(x, y, z));
}

void Chunk::fillSkyLight(uint8_t level) {
    skyLight.fill(level);
    dirty = true;
}

void Chunk::fillBlockLight(uint8_t level) {
    blockLight.fill(level);
    dirty = true;
}

} // namespace PoorCraft

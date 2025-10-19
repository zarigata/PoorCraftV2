#include "poorcraft/network/ChunkCompression.h"

#include <algorithm>
#include <limits>

#include "poorcraft/world/Chunk.h"

namespace PoorCraft {

std::vector<std::uint8_t> ChunkCompression::compressChunk(const Chunk& chunk) {
    return encodeRLE(chunk);
}

bool ChunkCompression::decompressChunk(const std::vector<std::uint8_t>& data, Chunk& chunk) {
    return decodeRLE(data, chunk);
}

float ChunkCompression::estimateCompressionRatio(const Chunk& chunk) {
    (void)chunk;
    return 1.0f;
}

std::vector<std::uint8_t> ChunkCompression::encodeRLE(const Chunk& chunk) {
    std::vector<std::uint8_t> output;
    output.reserve(Chunk::CHUNK_VOLUME * 2);

    uint16_t currentBlock = chunk.getBlock(0, 0, 0);
    uint16_t runLength = 1;

    for (int32_t i = 1; i < Chunk::CHUNK_VOLUME; ++i) {
        int32_t x = i % Chunk::CHUNK_SIZE_X;
        int32_t z = (i / Chunk::CHUNK_SIZE_X) % Chunk::CHUNK_SIZE_Z;
        int32_t y = i / (Chunk::CHUNK_SIZE_X * Chunk::CHUNK_SIZE_Z);

        uint16_t block = chunk.getBlock(x, y, z);
        if (block == currentBlock && runLength < std::numeric_limits<uint16_t>::max()) {
            ++runLength;
        } else {
            output.push_back(static_cast<std::uint8_t>(currentBlock & 0xFF));
            output.push_back(static_cast<std::uint8_t>((currentBlock >> 8) & 0xFF));
            output.push_back(static_cast<std::uint8_t>(runLength & 0xFF));
            output.push_back(static_cast<std::uint8_t>((runLength >> 8) & 0xFF));

            currentBlock = block;
            runLength = 1;
        }
    }

    output.push_back(static_cast<std::uint8_t>(currentBlock & 0xFF));
    output.push_back(static_cast<std::uint8_t>((currentBlock >> 8) & 0xFF));
    output.push_back(static_cast<std::uint8_t>(runLength & 0xFF));
    output.push_back(static_cast<std::uint8_t>((runLength >> 8) & 0xFF));

    return output;
}

bool ChunkCompression::decodeRLE(const std::vector<std::uint8_t>& data, Chunk& chunk) {
    if (data.size() % 4 != 0) {
        return false;
    }

    int32_t blockIndex = 0;
    for (std::size_t i = 0; i < data.size(); i += 4) {
        uint16_t blockId = static_cast<uint16_t>(data[i] | (data[i + 1] << 8));
        uint16_t runLength = static_cast<uint16_t>(data[i + 2] | (data[i + 3] << 8));

        for (uint16_t run = 0; run < runLength; ++run) {
            if (blockIndex >= Chunk::CHUNK_VOLUME) {
                return false;
            }

            int32_t x = blockIndex % Chunk::CHUNK_SIZE_X;
            int32_t z = (blockIndex / Chunk::CHUNK_SIZE_X) % Chunk::CHUNK_SIZE_Z;
            int32_t y = blockIndex / (Chunk::CHUNK_SIZE_X * Chunk::CHUNK_SIZE_Z);

            chunk.setBlock(x, y, z, blockId);
            ++blockIndex;
        }
    }

    return blockIndex == Chunk::CHUNK_VOLUME;
}

} // namespace PoorCraft

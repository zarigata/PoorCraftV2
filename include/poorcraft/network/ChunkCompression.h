#pragma once

#include <cstdint>
#include <vector>

namespace PoorCraft {

class Chunk;

class ChunkCompression {
public:
    static std::vector<std::uint8_t> compressChunk(const Chunk& chunk);
    static bool decompressChunk(const std::vector<std::uint8_t>& data, Chunk& chunk);
    static float estimateCompressionRatio(const Chunk& chunk);

private:
    static std::vector<std::uint8_t> encodeRLE(const Chunk& chunk);
    static bool decodeRLE(const std::vector<std::uint8_t>& data, Chunk& chunk);
};

} // namespace PoorCraft

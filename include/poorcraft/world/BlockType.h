#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace PoorCraft {

enum class BlockFace : uint8_t {
    FRONT = 0,
    BACK = 1,
    LEFT = 2,
    RIGHT = 3,
    TOP = 4,
    BOTTOM = 5
};

struct BlockType {
    uint16_t id;
    std::string name;
    bool isSolid;
    bool isOpaque;
    bool isTransparent;
    std::array<std::string, 6> textureIndices;
    uint8_t lightEmission;
    float hardness;

    BlockType();

    BlockType& setId(uint16_t newId);
    BlockType& setName(const std::string& newName);
    BlockType& setSolid(bool solid);
    BlockType& setOpaque(bool opaque);
    BlockType& setTransparent(bool transparent);
    BlockType& setTextureAllFaces(const std::string& textureName);
    BlockType& setTextureForFace(BlockFace face, const std::string& textureName);
    BlockType& setTexturePerFace(const std::array<std::string, 6>& textures);
    BlockType& setLightEmission(uint8_t emission);
    BlockType& setHardness(float blockHardness);

    [[nodiscard]] const std::string& getTextureName(BlockFace face) const;
};

} // namespace PoorCraft

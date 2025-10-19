#include "poorcraft/world/BlockType.h"

namespace PoorCraft {

BlockType::BlockType()
    : id(0), name(), isSolid(true), isOpaque(true), isTransparent(false), isLiquid(false), textureIndices(),
      lightEmission(0), hardness(1.0f) {}

BlockType& BlockType::setId(uint16_t newId) {
    id = newId;
    return *this;
}

BlockType& BlockType::setName(const std::string& newName) {
    name = newName;
    return *this;
}

BlockType& BlockType::setSolid(bool solid) {
    isSolid = solid;
    return *this;
}

BlockType& BlockType::setOpaque(bool opaque) {
    isOpaque = opaque;
    return *this;
}

BlockType& BlockType::setTransparent(bool transparent) {
    isTransparent = transparent;
    return *this;
}

BlockType& BlockType::setLiquid(bool liquid) {
    isLiquid = liquid;
    return *this;
}

BlockType& BlockType::setTextureAllFaces(const std::string& textureName) {
    textureIndices.fill(textureName);
    return *this;
}

BlockType& BlockType::setTextureForFace(BlockFace face, const std::string& textureName) {
    textureIndices[static_cast<size_t>(face)] = textureName;
    return *this;
}

BlockType& BlockType::setTexturePerFace(const std::array<std::string, 6>& textures) {
    textureIndices = textures;
    return *this;
}

BlockType& BlockType::setLightEmission(uint8_t emission) {
    lightEmission = emission;
    return *this;
}

BlockType& BlockType::setHardness(float blockHardness) {
    hardness = blockHardness;
    return *this;
}

const std::string& BlockType::getTextureName(BlockFace face) const {
    const auto& texture = textureIndices[static_cast<size_t>(face)];
    if (!texture.empty()) {
        return texture;
    }

    return textureIndices[0];
}

} // namespace PoorCraft

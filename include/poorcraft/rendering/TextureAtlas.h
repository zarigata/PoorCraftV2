#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glad/glad.h>

#include "poorcraft/core/Logger.h"
#include "poorcraft/rendering/Texture.h"

namespace PoorCraft {

struct AtlasEntry {
    std::string textureName;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
};

class TextureAtlas {
public:
    explicit TextureAtlas(int atlasSize = 2048, TextureFormat format = TextureFormat::RGBA);
    TextureAtlas(int width, int height, TextureFormat format = TextureFormat::RGBA);

    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;
    TextureAtlas(TextureAtlas&&) = default;
    TextureAtlas& operator=(TextureAtlas&&) = default;

    bool addTexture(const std::string& name, int width, int height, const uint8_t* data);
    bool addTextureFromFile(const std::string& name, const std::string& path);

    bool build();

    const AtlasEntry* getEntry(const std::string& name) const;
    std::shared_ptr<Texture> getTexture() const;

    std::pair<int, int> getAtlasSize() const;
    size_t getEntryCount() const;

private:
    bool packTexture(int width, int height, int& outX, int& outY);
    int getChannelCount(TextureFormat format) const;

private:
    int m_AtlasWidth = 0;
    int m_AtlasHeight = 0;
    TextureFormat m_Format = TextureFormat::RGBA;

    int m_CurrentX = 0;
    int m_CurrentY = 0;
    int m_CurrentRowHeight = 0;

    std::unordered_map<std::string, AtlasEntry> m_Entries;
    std::shared_ptr<Texture> m_AtlasTexture;
    std::vector<uint8_t> m_Buffer;
};

} // namespace PoorCraft

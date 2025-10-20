#include "poorcraft/rendering/TextureAtlas.h"

#include <algorithm>
#include <cstring>

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"

#include "stb_image.h"

namespace PoorCraft {

TextureAtlas::TextureAtlas(int atlasSize, TextureFormat format)
    : TextureAtlas(atlasSize, atlasSize, format) {}

TextureAtlas::TextureAtlas(int width, int height, TextureFormat format)
    : m_AtlasWidth(width), m_AtlasHeight(height), m_Format(format) {
    const int channels = getChannelCount(m_Format);
    m_Buffer.resize(static_cast<size_t>(m_AtlasWidth) * static_cast<size_t>(m_AtlasHeight) * static_cast<size_t>(channels), 0);
    PC_INFOF("TextureAtlas created (%dx%d, format channels: %d)", m_AtlasWidth, m_AtlasHeight, channels);
}

bool TextureAtlas::addTexture(const std::string& name, int width, int height, const uint8_t* data) {
    if (width <= 0 || height <= 0 || data == nullptr) {
        PC_ERRORF("Invalid texture dimensions or data for '%s'", name.c_str());
        return false;
    }

    if (m_Entries.find(name) != m_Entries.end()) {
        PC_WARNF("Texture '%s' already exists in atlas", name.c_str());
        return false;
    }

    int x = 0;
    int y = 0;
    if (!packTexture(width, height, x, y)) {
        PC_ERRORF("Texture '%s' does not fit in atlas", name.c_str());
        return false;
    }

    const int channels = getChannelCount(m_Format);
    for (int row = 0; row < height; ++row) {
        std::memcpy(&m_Buffer[((y + row) * m_AtlasWidth + x) * channels], &data[row * width * channels], static_cast<size_t>(width) * static_cast<size_t>(channels));
    }

    AtlasEntry entry{};
    entry.textureName = name;
    entry.x = x;
    entry.y = y;
    entry.width = width;
    entry.height = height;
    entry.u0 = static_cast<float>(x) / static_cast<float>(m_AtlasWidth);
    entry.v0 = static_cast<float>(y) / static_cast<float>(m_AtlasHeight);
    entry.u1 = static_cast<float>(x + width) / static_cast<float>(m_AtlasWidth);
    entry.v1 = static_cast<float>(y + height) / static_cast<float>(m_AtlasHeight);

    m_Entries.emplace(name, entry);

    PC_INFOF("Texture '%s' added to atlas at (%d, %d) size (%d x %d)", name.c_str(), x, y, width, height);

    return true;
}

bool TextureAtlas::addTextureFromFile(const std::string& name, const std::string& path) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(false);
    uint8_t* data = stbi_load(path.c_str(), &width, &height, &channels, getChannelCount(m_Format));
    if (!data) {
        PC_ERRORF("Failed to load texture '%s': %s", path.c_str(), stbi_failure_reason());
        return false;
    }

    bool result = addTexture(name, width, height, data);
    stbi_image_free(data);
    return result;
}

bool TextureAtlas::build() {
    if (!m_Buffer.empty()) {
        m_AtlasTexture = Texture::createFromData(m_AtlasWidth, m_AtlasHeight, m_Format, m_Buffer.data());
        m_Buffer.clear();
        m_Buffer.shrink_to_fit();
    }

    if (!m_AtlasTexture) {
        PC_ERROR("Failed to build texture atlas texture");
        return false;
    }

    m_AtlasTexture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Enable anisotropic filtering if supported
    if (GLAD_GL_EXT_texture_filter_anisotropic) {
        auto& config = poorcraft::Config::get_instance();
        const bool enableAniso = config.get_bool(poorcraft::Config::RenderingConfig::ENABLE_ANISOTROPIC_FILTERING_KEY, true);
        
        if (enableAniso) {
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            
            // Clamp to config value
            const float configMaxAniso = config.get_float(poorcraft::Config::RenderingConfig::MAX_ANISOTROPY_KEY, 16.0f);
            maxAniso = std::min(maxAniso, configMaxAniso);
            
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
            PC_DEBUGF("Texture atlas anisotropic filtering enabled: %.1fx", maxAniso);
        }
    }
    
    Texture::unbind();

    PC_INFOF("Texture atlas built with %zu textures", m_Entries.size());
    return true;
}

const AtlasEntry* TextureAtlas::getEntry(const std::string& name) const {
    auto it = m_Entries.find(name);
    if (it == m_Entries.end()) {
        return nullptr;
    }
    return &it->second;
}

std::shared_ptr<Texture> TextureAtlas::getTexture() const {
    return m_AtlasTexture;
}

std::pair<int, int> TextureAtlas::getAtlasSize() const {
    return {m_AtlasWidth, m_AtlasHeight};
}

size_t TextureAtlas::getEntryCount() const {
    return m_Entries.size();
}

bool TextureAtlas::packTexture(int width, int height, int& outX, int& outY) {
    if (m_CurrentX + width > m_AtlasWidth) {
        m_CurrentX = 0;
        m_CurrentY += m_CurrentRowHeight;
        m_CurrentRowHeight = 0;
    }

    if (m_CurrentY + height > m_AtlasHeight) {
        return false;
    }

    outX = m_CurrentX;
    outY = m_CurrentY;

    m_CurrentX += width;
    m_CurrentRowHeight = std::max(m_CurrentRowHeight, height);

    return true;
}

int TextureAtlas::getChannelCount(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB:
            return 3;
        case TextureFormat::RGBA:
            return 4;
        case TextureFormat::RED:
            return 1;
        case TextureFormat::RG:
            return 2;
        case TextureFormat::DEPTH:
        case TextureFormat::DEPTH_STENCIL:
            return 1;
        default:
            return 4;
    }
}

} // namespace PoorCraft

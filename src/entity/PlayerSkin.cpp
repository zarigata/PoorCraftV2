#include "poorcraft/entity/PlayerSkin.h"

#include <array>

#include "poorcraft/core/Logger.h"
#include "stb_image.h"

namespace PoorCraft {

namespace {
constexpr int SKIN_WIDTH = 64;
constexpr int SKIN_HEIGHT = 32;
}

const PlayerSkin::FaceRegion& PlayerSkin::Region::getFace(PlayerSkin::Face face) const {
    switch (face) {
        case PlayerSkin::Face::Front:
            return front;
        case PlayerSkin::Face::Back:
            return back;
        case PlayerSkin::Face::Left:
            return left;
        case PlayerSkin::Face::Right:
            return right;
        case PlayerSkin::Face::Top:
            return top;
        case PlayerSkin::Face::Bottom:
            return bottom;
    }

    return front;
}

PlayerSkin::PlayerSkin(const std::string& path)
    : Resource(path) {}

bool PlayerSkin::load() {
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(m_Path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        PC_ERROR("Failed to load player skin: " + m_Path);
        setState(ResourceState::Failed);
        return false;
    }

    if (!validateDimensions(width, height)) {
        PC_ERROR("Player skin has invalid dimensions: " + std::to_string(width) + "x" + std::to_string(height));
        stbi_image_free(data);
        setState(ResourceState::Failed);
        return false;
    }

    TextureParams params;
    params.minFilter = TextureFilter::NEAREST;
    params.magFilter = TextureFilter::NEAREST;
    params.wrapS = TextureWrap::CLAMP_TO_EDGE;
    params.wrapT = TextureWrap::CLAMP_TO_EDGE;
    params.generateMipmaps = false;

    m_Texture = Texture::createFromData(width, height, TextureFormat::RGBA, data, params);
    stbi_image_free(data);

    if (!m_Texture) {
        PC_ERROR("Failed to create texture for player skin: " + m_Path);
        setState(ResourceState::Failed);
        return false;
    }

    buildLayout();
    setState(ResourceState::Loaded);
    return true;
}

void PlayerSkin::unload() {
    if (m_Texture) {
        m_Texture->unload();
        m_Texture.reset();
    }
    setState(ResourceState::Unloaded);
}

ResourceType PlayerSkin::getType() const {
    return ResourceType::Texture;
}

std::shared_ptr<Texture> PlayerSkin::getTexture() const {
    return m_Texture;
}

const PlayerSkin::SkinLayout& PlayerSkin::getSkinLayout() const {
    return m_Layout;
}

bool PlayerSkin::validateDimensions(int width, int height) const {
    return width == SKIN_WIDTH && height == SKIN_HEIGHT;
}

void PlayerSkin::buildLayout() {
    auto toUV = [](float px, float py) {
        return glm::vec2(px / static_cast<float>(SKIN_WIDTH), py / static_cast<float>(SKIN_HEIGHT));
    };

    auto setRegion = [&](PlayerSkin::Region& region,
                         glm::vec2 frontMin, glm::vec2 frontMax,
                         glm::vec2 backMin, glm::vec2 backMax,
                         glm::vec2 leftMin, glm::vec2 leftMax,
                         glm::vec2 rightMin, glm::vec2 rightMax,
                         glm::vec2 topMin, glm::vec2 topMax,
                         glm::vec2 bottomMin, glm::vec2 bottomMax) {
        region.front = { frontMin, frontMax };
        region.back = { backMin, backMax };
        region.left = { leftMin, leftMax };
        region.right = { rightMin, rightMax };
        region.top = { topMin, topMax };
        region.bottom = { bottomMin, bottomMax };
    };

    setRegion(m_Layout.head,
              toUV(8.0f, 8.0f), toUV(16.0f, 16.0f),   // Front
              toUV(24.0f, 8.0f), toUV(32.0f, 16.0f), // Back
              toUV(16.0f, 8.0f), toUV(24.0f, 16.0f), // Left
              toUV(0.0f, 8.0f), toUV(8.0f, 16.0f),   // Right
              toUV(8.0f, 0.0f), toUV(16.0f, 8.0f),   // Top
              toUV(16.0f, 0.0f), toUV(24.0f, 8.0f)); // Bottom

    setRegion(m_Layout.body,
              toUV(20.0f, 20.0f), toUV(28.0f, 32.0f), // Front
              toUV(32.0f, 20.0f), toUV(40.0f, 32.0f), // Back
              toUV(28.0f, 20.0f), toUV(32.0f, 32.0f), // Left
              toUV(16.0f, 20.0f), toUV(20.0f, 32.0f), // Right
              toUV(20.0f, 16.0f), toUV(28.0f, 20.0f), // Top
              toUV(28.0f, 16.0f), toUV(36.0f, 20.0f)); // Bottom

    setRegion(m_Layout.rightArm,
              toUV(44.0f, 20.0f), toUV(48.0f, 32.0f), // Front
              toUV(52.0f, 20.0f), toUV(56.0f, 32.0f), // Back
              toUV(48.0f, 20.0f), toUV(52.0f, 32.0f), // Left
              toUV(40.0f, 20.0f), toUV(44.0f, 32.0f), // Right
              toUV(44.0f, 16.0f), toUV(48.0f, 20.0f), // Top
              toUV(48.0f, 16.0f), toUV(52.0f, 20.0f)); // Bottom

    // Classic 64x32 skins mirror left limbs from right limbs when undefined.
    m_Layout.leftArm = m_Layout.rightArm;

    setRegion(m_Layout.rightLeg,
              toUV(4.0f, 20.0f), toUV(8.0f, 32.0f),   // Front
              toUV(12.0f, 20.0f), toUV(16.0f, 32.0f), // Back
              toUV(8.0f, 20.0f), toUV(12.0f, 32.0f),  // Left
              toUV(0.0f, 20.0f), toUV(4.0f, 32.0f),   // Right
              toUV(4.0f, 16.0f), toUV(8.0f, 20.0f),   // Top
              toUV(8.0f, 16.0f), toUV(12.0f, 20.0f)); // Bottom

    m_Layout.leftLeg = m_Layout.rightLeg;
}

} // namespace PoorCraft

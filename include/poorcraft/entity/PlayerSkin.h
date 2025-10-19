#pragma once

#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "poorcraft/rendering/Texture.h"
#include "poorcraft/resource/Resource.h"

namespace PoorCraft {

class PlayerSkin : public Resource {
public:
    enum class Face {
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom
    };

    struct FaceRegion {
        glm::vec2 uvMin{0.0f};
        glm::vec2 uvMax{1.0f};
    };

    struct Region {
        FaceRegion front;
        FaceRegion back;
        FaceRegion left;
        FaceRegion right;
        FaceRegion top;
        FaceRegion bottom;

        const FaceRegion& getFace(Face face) const;
    };

    struct SkinLayout {
        Region head;
        Region body;
        Region leftArm;
        Region rightArm;
        Region leftLeg;
        Region rightLeg;
    };

    explicit PlayerSkin(const std::string& path);

    bool load() override;
    void unload() override;
    ResourceType getType() const override;

    std::shared_ptr<Texture> getTexture() const;
    const SkinLayout& getSkinLayout() const;

private:
    bool validateDimensions(int width, int height) const;
    void buildLayout();

    std::shared_ptr<Texture> m_Texture;
    SkinLayout m_Layout;
};

} // namespace PoorCraft

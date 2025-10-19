#pragma once

#include <memory>
#include <vector>

#include "poorcraft/entity/Component.h"
#include "poorcraft/rendering/Texture.h"
#include "poorcraft/rendering/VertexArray.h"

namespace PoorCraft {

class Renderable : public Component {
public:
    struct Section {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
    };

    Renderable(std::shared_ptr<VertexArray> mesh,
               std::shared_ptr<Texture> texture,
               std::vector<Section> sections = {});

    const std::shared_ptr<VertexArray>& getMesh() const;
    const std::shared_ptr<Texture>& getTexture() const;
    const std::vector<Section>& getSections() const;
    void setSections(std::vector<Section> sections);

    bool isVisible() const;
    void setVisible(bool visible);

    bool getCastsShadow() const;
    void setCastsShadow(bool castsShadow);

    int getRenderLayer() const;
    void setRenderLayer(int layer);

private:
    std::shared_ptr<VertexArray> m_Mesh;
    std::shared_ptr<Texture> m_Texture;
    std::vector<Section> m_Sections;
    bool m_Visible = true;
    bool m_CastsShadow = true;
    int m_RenderLayer = 0;
};

} // namespace PoorCraft

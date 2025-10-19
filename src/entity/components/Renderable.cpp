#include "poorcraft/entity/components/Renderable.h"

namespace PoorCraft {

Renderable::Renderable(std::shared_ptr<VertexArray> mesh,
                       std::shared_ptr<Texture> texture,
                       std::vector<Section> sections)
    : m_Mesh(std::move(mesh)),
      m_Texture(std::move(texture)),
      m_Sections(std::move(sections)) {
}

const std::shared_ptr<VertexArray>& Renderable::getMesh() const {
    return m_Mesh;
}

const std::shared_ptr<Texture>& Renderable::getTexture() const {
    return m_Texture;
}

const std::vector<Renderable::Section>& Renderable::getSections() const {
    return m_Sections;
}

void Renderable::setSections(std::vector<Section> sections) {
    m_Sections = std::move(sections);
}

bool Renderable::isVisible() const {
    return m_Visible;
}

void Renderable::setVisible(bool visible) {
    m_Visible = visible;
}

bool Renderable::getCastsShadow() const {
    return m_CastsShadow;
}

void Renderable::setCastsShadow(bool castsShadow) {
    m_CastsShadow = castsShadow;
}

int Renderable::getRenderLayer() const {
    return m_RenderLayer;
}

void Renderable::setRenderLayer(int layer) {
    m_RenderLayer = layer;
}

} // namespace PoorCraft

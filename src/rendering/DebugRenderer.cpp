#include "poorcraft/rendering/DebugRenderer.h"

#include <array>

#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/rendering/Shader.h"

namespace PoorCraft {

namespace {
constexpr glm::vec4 toVec4(const glm::vec3& color, float alpha = 1.0f) {
    return glm::vec4(color, alpha);
}

std::array<glm::vec3, 8> getAABBCorners(const PhysicsAABB& bounds) {
    return bounds.getCorners();
}

void drawAABBEdges(Renderer& renderer,
                   Shader& shader,
                   const PhysicsAABB& bounds,
                   const glm::vec3& color) {
    const auto corners = getAABBCorners(bounds);
    const glm::vec4 color4 = toVec4(color);
    renderer.drawLine(corners[0], corners[1], color4, shader);
    renderer.drawLine(corners[1], corners[3], color4, shader);
    renderer.drawLine(corners[3], corners[2], color4, shader);
    renderer.drawLine(corners[2], corners[0], color4, shader);
    renderer.drawLine(corners[4], corners[5], color4, shader);
    renderer.drawLine(corners[5], corners[7], color4, shader);
    renderer.drawLine(corners[7], corners[6], color4, shader);
    renderer.drawLine(corners[6], corners[4], color4, shader);
    renderer.drawLine(corners[0], corners[4], color4, shader);
    renderer.drawLine(corners[1], corners[5], color4, shader);
    renderer.drawLine(corners[2], corners[6], color4, shader);
    renderer.drawLine(corners[3], corners[7], color4, shader);
}

} // namespace

DebugRenderer& DebugRenderer::getInstance() {
    static DebugRenderer instance;
    return instance;
}

void DebugRenderer::initialize() {
    if (m_Initialized) {
        return;
    }

    m_AABBs.clear();
    m_Rays.clear();
    m_Initialized = true;
}

void DebugRenderer::shutdown() {
    if (!m_Initialized) {
        return;
    }

    m_AABBs.clear();
    m_Rays.clear();
    m_Initialized = false;
}

void DebugRenderer::setEnabled(bool enabled) {
    ensureInitialized();
    m_Enabled = enabled;
}

bool DebugRenderer::isEnabled() const {
    return m_Enabled;
}

void DebugRenderer::clear() {
    if (!m_Initialized) {
        return;
    }

    m_AABBs.clear();
    m_Rays.clear();
}

void DebugRenderer::drawAABB(const PhysicsAABB& bounds, const glm::vec3& color) {
    ensureInitialized();
    if (!m_Enabled) {
        return;
    }

    m_AABBs.push_back(DebugAABB{bounds, color});
}

void DebugRenderer::drawRay(const glm::vec3& origin,
                            const glm::vec3& direction,
                            float length,
                            const glm::vec3& color) {
    ensureInitialized();
    if (!m_Enabled || length <= 0.0f) {
        return;
    }

    DebugRay ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.length = length;
    ray.color = color;
    m_Rays.push_back(ray);
}

void DebugRenderer::render(const Camera& camera) {
    if (!m_Initialized || !m_Enabled) {
        return;
    }

    Renderer& renderer = Renderer::getInstance();
    auto shaderPtr = renderer.getDefaultShader();
    if (!shaderPtr) {
        return;
    }

    Shader& shader = *shaderPtr;

    for (const auto& aabb : m_AABBs) {
        drawAABBEdges(renderer, shader, aabb.bounds, aabb.color);
    }

    for (const auto& ray : m_Rays) {
        const float dirLenSq = glm::length2(ray.direction);
        if (dirLenSq <= 0.0f) {
            continue;
        }

        const glm::vec3 end = ray.origin + glm::normalize(ray.direction) * ray.length;
        renderer.drawLine(ray.origin, end, toVec4(ray.color), shader);
    }
}

void DebugRenderer::ensureInitialized() {
    if (!m_Initialized) {
        initialize();
    }
}

} // namespace PoorCraft

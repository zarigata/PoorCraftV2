#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/Texture.h"
#include "poorcraft/rendering/VertexArray.h"

namespace PoorCraft {

struct RenderStats {
    uint32_t drawCalls = 0;
    uint32_t vertices = 0;
    uint32_t triangles = 0;
    uint32_t textureBinds = 0;
    uint32_t shaderBinds = 0;
};

class Renderer {
public:
    static Renderer& getInstance();

    bool initialize();
    void shutdown();

    void beginFrame();
    void endFrame();

    void clear() const;
    void setClearColor(const glm::vec4& color);

    void setViewport(int x, int y, int width, int height) const;
    void setDepthTest(bool enabled) const;
    void setBlending(bool enabled) const;
    void setCulling(bool enabled, GLenum face = GL_BACK) const;
    void setWireframe(bool enabled) const;

    void drawQuad(const glm::mat4& model,
                  const glm::vec4& color,
                  const std::shared_ptr<Texture>& texture,
                  Shader& shader);
    void drawCube(const glm::mat4& model,
                  const glm::vec4& color,
                  const std::shared_ptr<Texture>& texture,
                  Shader& shader);
    void drawLine(const glm::vec3& start,
                  const glm::vec3& end,
                  const glm::vec4& color,
                  Shader& shader);
    void drawGrid(int halfSize,
                  float spacing,
                  const glm::vec4& color,
                  Shader& shader);

    void setCamera(const Camera* camera);

    const RenderStats& getStats() const;
    void resetStats();

    std::shared_ptr<Shader> getDefaultShader() const;
    std::shared_ptr<Texture> getDefaultTexture() const;

private:
    Renderer() = default;
    ~Renderer() = default;

    void createDefaultResources();
    void destroyDefaultResources();

    std::shared_ptr<VertexArray> createQuadVAO();
    std::shared_ptr<VertexArray> createCubeVAO();
    std::shared_ptr<VertexArray> createLineVAO();
    void applyCameraUniforms(Shader& shader) const;

private:
    glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    RenderStats m_Stats;

    std::shared_ptr<Shader> m_DefaultShader;
    std::shared_ptr<Texture> m_DefaultTexture;

    std::shared_ptr<VertexArray> m_QuadVAO;
    std::shared_ptr<VertexArray> m_CubeVAO;
    std::shared_ptr<VertexArray> m_LineVAO;
    std::size_t m_LineVBOIndex = static_cast<std::size_t>(-1);

    const Camera* m_ActiveCamera = nullptr;
};

} // namespace PoorCraft

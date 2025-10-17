#include "poorcraft/rendering/Renderer.h"

#include <array>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "poorcraft/core/Logger.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/rendering/GPUCapabilities.h"

namespace PoorCraft {

namespace {
constexpr glm::vec4 DEFAULT_CLEAR_COLOR(0.1f, 0.1f, 0.1f, 1.0f);

struct QuadVertex {
    glm::vec3 position;
    glm::vec2 uv;
};

struct CubeVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct LineVertex {
    glm::vec3 position;
};

} // namespace

Renderer& Renderer::getInstance() {
    static Renderer instance;
    return instance;
}

bool Renderer::initialize() {
    PC_INFO("Renderer initialization started");

    if (!GPUCapabilities::getInstance().query()) {
        PC_WARN("GPU capability query failed; continuing with defaults");
    }

    createDefaultResources();

    m_ClearColor = DEFAULT_CLEAR_COLOR;
    glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);

    setDepthTest(true);
    setBlending(true);
    setCulling(true);

    m_QuadVAO = createQuadVAO();
    m_CubeVAO = createCubeVAO();
    m_LineVAO = createLineVAO();

    PC_INFO("Renderer initialization complete");
    return true;
}

void Renderer::shutdown() {
    PC_INFO("Renderer shutting down");

    m_QuadVAO.reset();
    m_CubeVAO.reset();
    m_LineVAO.reset();

    destroyDefaultResources();

    PC_INFO("Renderer shutdown complete");
}

void Renderer::beginFrame() {
    m_Stats = {};
}

void Renderer::endFrame() {
    PC_TRACEF("Render stats - DrawCalls: %u, Vertices: %u, Triangles: %u",
              m_Stats.drawCalls,
              m_Stats.vertices,
              m_Stats.triangles);
}

void Renderer::clear() const {
    glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setClearColor(const glm::vec4& color) {
    m_ClearColor = color;
    glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
}

void Renderer::setViewport(int x, int y, int width, int height) const {
    glViewport(x, y, width, height);
}

void Renderer::setDepthTest(bool enabled) const {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::setBlending(bool enabled) const {
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}

void Renderer::setCulling(bool enabled, GLenum face) const {
    if (enabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(face);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void Renderer::setWireframe(bool enabled) const {
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void Renderer::drawQuad(const glm::mat4& model,
                        const glm::vec4& color,
                        const std::shared_ptr<Texture>& texture,
                        Shader& shader) {
    if (!m_QuadVAO) {
        PC_WARN("Attempted to draw quad without VAO");
        return;
    }

    shader.bind();
    shader.setMat4("model", model);
    shader.setVec4("color", color);

    if (texture) {
        texture->bind(0);
        shader.setInt("textureSampler", 0);
        ++m_Stats.textureBinds;
    } else if (m_DefaultTexture) {
        m_DefaultTexture->bind(0);
        shader.setInt("textureSampler", 0);
        ++m_Stats.textureBinds;
    }

    ++m_Stats.shaderBinds;

    m_QuadVAO->bind();
    if (m_QuadVAO->hasIndices()) {
        m_QuadVAO->draw(GL_TRIANGLES, m_QuadVAO->getIndexCount());
    } else {
        m_QuadVAO->draw(GL_TRIANGLES, 6);
    }
    VertexArray::unbind();

    ++m_Stats.drawCalls;
    m_Stats.vertices += 6;
    m_Stats.triangles += 2;
}

void Renderer::drawCube(const glm::mat4& model,
                        const glm::vec4& color,
                        const std::shared_ptr<Texture>& texture,
                        Shader& shader) {
    if (!m_CubeVAO) {
        PC_WARN("Attempted to draw cube without VAO");
        return;
    }

    shader.bind();
    shader.setMat4("model", model);
    shader.setVec4("color", color);

    if (texture) {
        texture->bind(0);
        shader.setInt("textureSampler", 0);
        ++m_Stats.textureBinds;
    } else if (m_DefaultTexture) {
        m_DefaultTexture->bind(0);
        shader.setInt("textureSampler", 0);
        ++m_Stats.textureBinds;
    }

    ++m_Stats.shaderBinds;

    m_CubeVAO->bind();
    if (m_CubeVAO->hasIndices()) {
        m_CubeVAO->draw(GL_TRIANGLES, m_CubeVAO->getIndexCount());
    } else {
        m_CubeVAO->draw(GL_TRIANGLES, 36);
    }
    VertexArray::unbind();

    ++m_Stats.drawCalls;
    m_Stats.vertices += 36;
    m_Stats.triangles += 12;
}

void Renderer::drawLine(const glm::vec3& start,
                        const glm::vec3& end,
                        const glm::vec4& color,
                        Shader& shader) {
    if (!m_LineVAO) {
        PC_WARN("Attempted to draw line without VAO");
        return;
    }

    std::array<LineVertex, 2> vertices = {
        LineVertex{start},
        LineVertex{end}
    };

    if (m_LineVBOIndex != static_cast<std::size_t>(-1)) {
        m_LineVAO->updateVertexBuffer(m_LineVBOIndex, 0, vertices.data(), sizeof(LineVertex) * vertices.size());
    }

    shader.bind();
    shader.setVec4("color", color);
    ++m_Stats.shaderBinds;

    m_LineVAO->bind();
    m_LineVAO->draw(GL_LINES, 2);
    VertexArray::unbind();

    ++m_Stats.drawCalls;
    m_Stats.vertices += 2;
}

void Renderer::drawGrid(int halfSize,
                        float spacing,
                        const glm::vec4& color,
                        Shader& shader) {
    for (int i = -halfSize; i <= halfSize; ++i) {
        glm::vec3 startX(-halfSize * spacing, 0.0f, static_cast<float>(i) * spacing);
        glm::vec3 endX(halfSize * spacing, 0.0f, static_cast<float>(i) * spacing);
        drawLine(startX, endX, color, shader);

        glm::vec3 startZ(static_cast<float>(i) * spacing, 0.0f, -halfSize * spacing);
        glm::vec3 endZ(static_cast<float>(i) * spacing, 0.0f, halfSize * spacing);
        drawLine(startZ, endZ, color, shader);
    }
}

const RenderStats& Renderer::getStats() const {
    return m_Stats;
}

void Renderer::resetStats() {
    m_Stats = {};
}

std::shared_ptr<Shader> Renderer::getDefaultShader() const {
    return m_DefaultShader;
}

std::shared_ptr<Texture> Renderer::getDefaultTexture() const {
    return m_DefaultTexture;
}

void Renderer::createDefaultResources() {
    static const uint8_t whitePixel[4] = {255, 255, 255, 255};

    TextureParams texParams;
    texParams.generateMipmaps = false;
    texParams.minFilter = TextureFilter::LINEAR;
    texParams.magFilter = TextureFilter::LINEAR;
    texParams.wrapS = TextureWrap::CLAMP_TO_EDGE;
    texParams.wrapT = TextureWrap::CLAMP_TO_EDGE;

    m_DefaultTexture = Texture::createFromData(1, 1, TextureFormat::RGBA, whitePixel, texParams);

    std::string shaderBase = poorcraft::Platform::join_path("shaders", "basic/texture");
    if (!poorcraft::Platform::is_absolute_path(shaderBase)) {
        shaderBase = poorcraft::Platform::join_path(poorcraft::Platform::get_current_working_directory(), shaderBase);
    }

    auto shader = std::make_shared<Shader>(shaderBase);
    if (shader->load()) {
        m_DefaultShader = shader;
    } else {
        PC_ERRORF("Failed to load default shader from '%s'", shaderBase.c_str());
        m_DefaultShader.reset();
    }
}

void Renderer::destroyDefaultResources() {
    if (m_DefaultShader) {
        m_DefaultShader->unload();
        m_DefaultShader.reset();
    }

    if (m_DefaultTexture) {
        m_DefaultTexture->unload();
        m_DefaultTexture.reset();
    }
}

std::shared_ptr<VertexArray> Renderer::createQuadVAO() {
    auto vao = std::make_shared<VertexArray>();

    std::array<QuadVertex, 4> vertices = {
        QuadVertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
        QuadVertex{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
        QuadVertex{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
        QuadVertex{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}}
    };

    std::array<uint32_t, 6> indices = {0, 1, 2, 2, 3, 0};

    std::vector<VertexAttribute> attributes = {
        VertexAttribute{0, 3, VertexAttributeType::FLOAT, false, sizeof(QuadVertex), offsetof(QuadVertex, position)},
        VertexAttribute{1, 2, VertexAttributeType::FLOAT, false, sizeof(QuadVertex), offsetof(QuadVertex, uv)}
    };

    vao->addVertexBuffer(vertices.data(), sizeof(vertices), attributes, BufferUsage::STATIC_DRAW);
    vao->setIndexBuffer(indices.data(), indices.size(), BufferUsage::STATIC_DRAW);

    return vao;
}

std::shared_ptr<VertexArray> Renderer::createCubeVAO() {
    auto vao = std::make_shared<VertexArray>();

    std::array<CubeVertex, 36> vertices = {
        CubeVertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        CubeVertex{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        CubeVertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        CubeVertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        CubeVertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        CubeVertex{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

        CubeVertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        CubeVertex{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        CubeVertex{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        CubeVertex{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        CubeVertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        CubeVertex{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

        CubeVertex{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        CubeVertex{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        CubeVertex{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        CubeVertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        CubeVertex{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        CubeVertex{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        CubeVertex{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        CubeVertex{{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        CubeVertex{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

        CubeVertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        CubeVertex{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        CubeVertex{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        CubeVertex{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        CubeVertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
    };

    std::vector<VertexAttribute> attributes = {
        VertexAttribute{0, 3, VertexAttributeType::FLOAT, false, sizeof(CubeVertex), offsetof(CubeVertex, position)},
        VertexAttribute{1, 3, VertexAttributeType::FLOAT, false, sizeof(CubeVertex), offsetof(CubeVertex, normal)},
        VertexAttribute{2, 2, VertexAttributeType::FLOAT, false, sizeof(CubeVertex), offsetof(CubeVertex, uv)}
    };

    vao->addVertexBuffer(vertices.data(), sizeof(vertices), attributes, BufferUsage::STATIC_DRAW);

    return vao;
}

std::shared_ptr<VertexArray> Renderer::createLineVAO() {
    auto vao = std::make_shared<VertexArray>();

    std::array<LineVertex, 2> vertices = {
        LineVertex{{0.0f, 0.0f, 0.0f}},
        LineVertex{{0.0f, 0.0f, 0.0f}}
    };

    std::vector<VertexAttribute> attributes = {
        VertexAttribute{0, 3, VertexAttributeType::FLOAT, false, sizeof(LineVertex), offsetof(LineVertex, position)}
    };

    m_LineVBOIndex = vao->addVertexBuffer(vertices.data(), sizeof(vertices), attributes, BufferUsage::DYNAMIC_DRAW);

    return vao;
}

} // namespace PoorCraft

#pragma once

#include "poorcraft/rendering/RenderBackend.h"

namespace PoorCraft {

// Forward declarations
class Window;

/**
 * OpenGL rendering backend implementation
 * Wraps existing Renderer singleton to work through IRenderBackend interface
 * Maintains backward compatibility with existing OpenGL rendering code
 */
class OpenGLBackend : public IRenderBackend {
public:
    explicit OpenGLBackend(Window& window);
    ~OpenGLBackend() override = default;

    // Lifecycle
    bool initialize() override;
    void shutdown() override;

    // Frame operations
    void beginFrame() override;
    void endFrame() override;
    void clear() override;
    void setClearColor(float r, float g, float b, float a) override;
    void setViewport(int x, int y, int width, int height) override;

    // High-level rendering
    void renderWorld(World& world, Camera& camera, float deltaTime) override;
    void renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) override;
    void renderUI() override;

    // Queries
    RenderBackendType getBackendType() const override { return RenderBackendType::OPENGL; }
    std::string getBackendName() const override { return "OpenGL 4.6"; }
    RenderStats getStats() const override;
    bool supportsRayTracing() const override { return false; }
    bool isInitialized() const override;

private:
    Window& m_Window;
    bool m_Initialized = false;
};

} // namespace PoorCraft

#include "poorcraft/rendering/OpenGLBackend.h"
#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/world/World.h"
#include "poorcraft/entity/EntityRenderer.h"
#include "poorcraft/ui/UIManager.h"
#include "poorcraft/core/Logger.h"

namespace PoorCraft {

OpenGLBackend::OpenGLBackend(Window& window)
    : m_Window(window) {
}

bool OpenGLBackend::initialize() {
    PC_INFO("Initializing OpenGL backend");
    
    bool success = Renderer::getInstance().initialize();
    if (success) {
        m_Initialized = true;
        PC_INFO("OpenGL backend initialized successfully");
    } else {
        PC_ERROR("Failed to initialize OpenGL backend");
    }
    
    return success;
}

void OpenGLBackend::shutdown() {
    PC_INFO("Shutting down OpenGL backend");
    Renderer::getInstance().shutdown();
    m_Initialized = false;
}

void OpenGLBackend::beginFrame() {
    Renderer::getInstance().beginFrame();
}

void OpenGLBackend::endFrame() {
    Renderer::getInstance().endFrame();
}

void OpenGLBackend::clear() {
    Renderer::getInstance().clear();
}

void OpenGLBackend::setClearColor(float r, float g, float b, float a) {
    Renderer::getInstance().setClearColor(r, g, b, a);
}

void OpenGLBackend::setViewport(int x, int y, int width, int height) {
    Renderer::getInstance().setViewport(x, y, width, height);
}

void OpenGLBackend::renderWorld(World& world, Camera& camera, float deltaTime) {
    // World rendering is handled by World::render() which uses existing OpenGL path
    // This is called from the main game loop
    (void)world;
    (void)camera;
    (void)deltaTime;
}

void OpenGLBackend::renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) {
    // Entity rendering is handled by EntityRenderer::render()
    // This is called from the main game loop
    (void)entityRenderer;
    (void)camera;
    (void)alpha;
}

void OpenGLBackend::renderUI() {
    // UI rendering is handled by UIManager::endFrame()
    // This is called from the main game loop
    UIManager::getInstance().endFrame();
}

RenderStats OpenGLBackend::getStats() const {
    return Renderer::getInstance().getStats();
}

bool OpenGLBackend::isInitialized() const {
    return m_Initialized;
}

} // namespace PoorCraft

#include "poorcraft/rendering/OpenGLBackend.h"
#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/world/World.h"
#include "poorcraft/entity/EntityRenderer.h"
#include "poorcraft/ui/UIManager.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/core/Logger.h"
#include <memory>

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
    // Load block shader if not already loaded
    static std::shared_ptr<Shader> blockShader = nullptr;
    static bool shaderLoaded = false;
    
    if (!shaderLoaded) {
        try {
            blockShader = ResourceManager::getInstance().load<Shader>("shaders/basic/block");
            if (!blockShader || !blockShader->isValid()) {
                PC_WARN("Failed to load block shader, using default");
                blockShader = Renderer::getInstance().getDefaultShader();
            }
        } catch (const std::exception& e) {
            PC_ERROR("Error loading block shader: {}", e.what());
            blockShader = Renderer::getInstance().getDefaultShader();
        }
        shaderLoaded = true;
    }
    
    if (blockShader) {
        blockShader->bind();
        world.render(camera, *blockShader);
    }
    
    (void)deltaTime; // Not currently used
}

void OpenGLBackend::renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) {
    // Load entity shader if not already loaded
    static std::shared_ptr<Shader> entityShader = nullptr;
    static bool shaderLoaded = false;
    
    if (!shaderLoaded) {
        try {
            entityShader = ResourceManager::getInstance().load<Shader>("shaders/basic/entity");
            if (!entityShader || !entityShader->isValid()) {
                PC_WARN("Failed to load entity shader, using default");
                entityShader = Renderer::getInstance().getDefaultShader();
            }
        } catch (const std::exception& e) {
            PC_ERROR("Error loading entity shader: {}", e.what());
            entityShader = Renderer::getInstance().getDefaultShader();
        }
        shaderLoaded = true;
    }
    
    if (entityShader) {
        entityShader->bind();
        entityRenderer.render(camera, *entityShader, alpha);
    }
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

#pragma once

#include <memory>
#include <string>

namespace PoorCraft {

// Forward declarations
class Window;
class World;
class Camera;
class EntityRenderer;

// Rendering backend types
enum class RenderBackendType {
    OPENGL,     // OpenGL 4.6 (default, maximum compatibility)
    VULKAN,     // Vulkan 1.3 (modern API, better performance)
    VULKAN_RT   // Vulkan + Ray Tracing (realistic lighting)
};

// Rendering statistics
struct RenderStats {
    uint32_t drawCalls = 0;
    uint32_t vertices = 0;
    uint32_t triangles = 0;
    float frameTime = 0.0f;
};

/**
 * Abstract rendering backend interface
 * Provides high-level rendering operations abstracted from specific graphics APIs
 * Implementations: OpenGLBackend, VulkanBackend
 */
class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    // Lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Frame operations
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void clear() = 0;
    virtual void setClearColor(float r, float g, float b, float a) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;

    // High-level rendering
    virtual void renderWorld(World& world, Camera& camera, float deltaTime) = 0;
    virtual void renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) = 0;
    virtual void renderUI() = 0;

    // Queries
    virtual RenderBackendType getBackendType() const = 0;
    virtual std::string getBackendName() const = 0;
    virtual RenderStats getStats() const = 0;
    virtual bool supportsRayTracing() const = 0;
    virtual bool isInitialized() const = 0;
};

/**
 * Factory for creating rendering backends
 * Handles backend selection and fallback logic
 */
class RenderBackendFactory {
public:
    static std::unique_ptr<IRenderBackend> create(RenderBackendType type, Window& window);
};

} // namespace PoorCraft

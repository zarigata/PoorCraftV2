#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/rendering/RenderBackend.h"
#include <memory>

namespace PoorCraft {

// Forward declarations
class VulkanContext;
class VulkanShaderManager;
class VulkanResourceManager;
class VulkanRasterRenderer;
class RTRenderer;
class Window;

/**
 * Vulkan rendering backend with optional ray tracing
 * Supports both raster and ray tracing rendering paths
 */
class VulkanBackend : public IRenderBackend {
public:
    VulkanBackend(Window& window, bool enableRayTracing);
    ~VulkanBackend() override;

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
    RenderBackendType getBackendType() const override;
    std::string getBackendName() const override { return m_RayTracingEnabled ? "Vulkan + Ray Tracing" : "Vulkan"; }
    RenderStats getStats() const override { return m_Stats; }
    bool supportsRayTracing() const override { return m_RayTracingEnabled; }
    bool isInitialized() const override { return m_Initialized; }

private:
    void handleSwapchainRecreation();
    bool initializeImGui();

    Window& m_Window;
    std::unique_ptr<VulkanContext> m_Context;
    std::unique_ptr<VulkanShaderManager> m_ShaderManager;
    std::unique_ptr<VulkanResourceManager> m_ResourceManager;
    std::unique_ptr<VulkanRasterRenderer> m_RasterRenderer;
    std::unique_ptr<RTRenderer> m_RTRenderer;
    
    VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;
    
    bool m_RayTracingEnabled;
    bool m_Initialized = false;
    
    RenderStats m_Stats;
    uint32_t m_CurrentImageIndex = 0;
    float m_ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

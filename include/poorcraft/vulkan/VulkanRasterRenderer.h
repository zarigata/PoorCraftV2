#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <memory>

namespace PoorCraft {

// Forward declarations
class VulkanContext;
class VulkanShaderManager;
class World;
class Camera;

/**
 * Vulkan rasterization renderer
 * Handles traditional rasterization pipeline for world and entity rendering
 */
class VulkanRasterRenderer {
public:
    VulkanRasterRenderer(VulkanContext& context, VulkanShaderManager& shaderManager);
    ~VulkanRasterRenderer();

    // Non-copyable
    VulkanRasterRenderer(const VulkanRasterRenderer&) = delete;
    VulkanRasterRenderer& operator=(const VulkanRasterRenderer&) = delete;

    /**
     * Initialize raster pipeline, render pass, and framebuffers
     */
    bool initialize();

    /**
     * Cleanup resources
     */
    void cleanup();

    /**
     * Begin render pass for current frame
     */
    void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /**
     * End render pass
     */
    void endRenderPass(VkCommandBuffer commandBuffer);

    /**
     * Render world geometry
     */
    void renderWorld(VkCommandBuffer commandBuffer, World& world, Camera& camera);

    /**
     * Get render pass handle
     */
    VkRenderPass getRenderPass() const { return m_RenderPass; }

    /**
     * Get framebuffer for specific swapchain image
     */
    VkFramebuffer getFramebuffer(uint32_t imageIndex) const { 
        return imageIndex < m_Framebuffers.size() ? m_Framebuffers[imageIndex] : VK_NULL_HANDLE; 
    }

private:
    bool createRenderPass();
    bool createFramebuffers();
    bool createGraphicsPipeline();
    void cleanupFramebuffers();

    VulkanContext& m_Context;
    VulkanShaderManager& m_ShaderManager;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_Framebuffers;

    bool m_Initialized = false;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

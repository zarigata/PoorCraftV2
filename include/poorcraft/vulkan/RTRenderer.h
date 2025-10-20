#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <memory>

namespace PoorCraft {

// Forward declarations
class VulkanContext;
class VulkanShaderManager;
class VulkanResourceManager;
class RTAccelerationStructure;
class World;
class Camera;

/**
 * Vulkan ray tracing renderer
 * Manages RT pipeline, shader binding table, and ray tracing dispatch
 */
class RTRenderer {
public:
    RTRenderer(VulkanContext& context, VulkanShaderManager& shaderManager, VulkanResourceManager& resourceManager);
    ~RTRenderer();

    // Non-copyable
    RTRenderer(const RTRenderer&) = delete;
    RTRenderer& operator=(const RTRenderer&) = delete;

    /**
     * Initialize RT pipeline and resources
     */
    bool initialize();

    /**
     * Cleanup resources
     */
    void cleanup();

    /**
     * Trace rays for the current frame
     */
    void traceRays(VkCommandBuffer commandBuffer, World& world, Camera& camera, uint32_t width, uint32_t height);

    /**
     * Get storage image view for compositing
     */
    VkImageView getStorageImageView() const { return m_StorageImageView; }

    /**
     * Composite RT output to swapchain
     */
    void composite(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t width, uint32_t height);

private:
    bool createRTPipeline();
    bool createShaderBindingTable();
    bool createDescriptorSets();
    bool createStorageImage(uint32_t width, uint32_t height);
    bool createUniformBuffers();
    bool createCompositingResources();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VulkanContext& m_Context;
    VulkanShaderManager& m_ShaderManager;
    VulkanResourceManager& m_ResourceManager;
    std::unique_ptr<RTAccelerationStructure> m_AccelStructure;

    VkPipeline m_RTPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

    // Shader binding table
    VkBuffer m_SBTBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_SBTMemory = VK_NULL_HANDLE;
    VkStridedDeviceAddressRegionKHR m_RaygenRegion{};
    VkStridedDeviceAddressRegionKHR m_MissRegion{};
    VkStridedDeviceAddressRegionKHR m_HitRegion{};
    VkStridedDeviceAddressRegionKHR m_CallableRegion{};

    // Storage image for RT output
    VkImage m_StorageImage = VK_NULL_HANDLE;
    VkDeviceMemory m_StorageImageMemory = VK_NULL_HANDLE;
    VkImageView m_StorageImageView = VK_NULL_HANDLE;
    
    // Camera uniform buffer
    VkBuffer m_CameraUBO = VK_NULL_HANDLE;
    VkDeviceMemory m_CameraUBOMemory = VK_NULL_HANDLE;

    // Compositing resources
    VkSampler m_Sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_CompositingDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_CompositingDescriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout m_CompositingPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_CompositingPipeline = VK_NULL_HANDLE;

    bool m_Initialized = false;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

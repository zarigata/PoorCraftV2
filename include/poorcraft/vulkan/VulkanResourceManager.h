#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <vector>

namespace PoorCraft {

class VulkanContext;

struct VulkanBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mappedPtr = nullptr;
};

struct VulkanImage {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format;
    VkExtent2D extent;
};

/**
 * Vulkan resource manager for buffers, images, and memory
 * Handles allocation, deallocation, and common operations
 */
class VulkanResourceManager {
public:
    explicit VulkanResourceManager(VulkanContext& context);
    ~VulkanResourceManager() = default;

    bool initialize();
    void shutdown();

    // Buffer operations
    VulkanBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyBuffer(VulkanBuffer& buffer);
    void* mapBuffer(VulkanBuffer& buffer);
    void unmapBuffer(VulkanBuffer& buffer);
    void uploadBufferData(VulkanBuffer& buffer, const void* data, VkDeviceSize size);

    // Image operations
    VulkanImage createImage(uint32_t width, uint32_t height, VkFormat format, 
                           VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyImage(VulkanImage& image);

private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VulkanContext& m_Context;
    std::vector<VulkanBuffer> m_Buffers;
    std::vector<VulkanImage> m_Images;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/vulkan/VulkanResourceManager.h"
#include "poorcraft/vulkan/VulkanContext.h"
#include "poorcraft/core/Logger.h"
#include <cstring>

namespace PoorCraft {

VulkanResourceManager::VulkanResourceManager(VulkanContext& context)
    : m_Context(context) {
}

bool VulkanResourceManager::initialize() {
    PC_INFO("Vulkan resource manager initialized");
    return true;
}

void VulkanResourceManager::shutdown() {
    // Cleanup all allocated resources
    for (auto& buffer : m_Buffers) {
        destroyBuffer(buffer);
    }
    m_Buffers.clear();

    for (auto& image : m_Images) {
        destroyImage(image);
    }
    m_Images.clear();

    PC_INFO("Vulkan resource manager shut down");
}

VulkanBuffer VulkanResourceManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                                                  VkMemoryPropertyFlags properties) {
    VulkanBuffer buffer;
    buffer.size = size;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Context.getDevice(), &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS) {
        PC_ERROR("Failed to create buffer");
        return buffer;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Context.getDevice(), buffer.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Context.getDevice(), &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate buffer memory");
        vkDestroyBuffer(m_Context.getDevice(), buffer.buffer, nullptr);
        buffer.buffer = VK_NULL_HANDLE;
        return buffer;
    }

    vkBindBufferMemory(m_Context.getDevice(), buffer.buffer, buffer.memory, 0);

    m_Buffers.push_back(buffer);
    return buffer;
}

void VulkanResourceManager::destroyBuffer(VulkanBuffer& buffer) {
    if (buffer.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_Context.getDevice(), buffer.buffer, nullptr);
        buffer.buffer = VK_NULL_HANDLE;
    }
    if (buffer.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_Context.getDevice(), buffer.memory, nullptr);
        buffer.memory = VK_NULL_HANDLE;
    }
}

void* VulkanResourceManager::mapBuffer(VulkanBuffer& buffer) {
    if (buffer.mappedPtr == nullptr) {
        vkMapMemory(m_Context.getDevice(), buffer.memory, 0, buffer.size, 0, &buffer.mappedPtr);
    }
    return buffer.mappedPtr;
}

void VulkanResourceManager::unmapBuffer(VulkanBuffer& buffer) {
    if (buffer.mappedPtr != nullptr) {
        vkUnmapMemory(m_Context.getDevice(), buffer.memory);
        buffer.mappedPtr = nullptr;
    }
}

void VulkanResourceManager::uploadBufferData(VulkanBuffer& buffer, const void* data, VkDeviceSize size) {
    void* mapped = mapBuffer(buffer);
    std::memcpy(mapped, data, static_cast<size_t>(size));
    unmapBuffer(buffer);
}

VulkanImage VulkanResourceManager::createImage(uint32_t width, uint32_t height, VkFormat format,
                                                VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    VulkanImage image;
    image.format = format;
    image.extent = {width, height};

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_Context.getDevice(), &imageInfo, nullptr, &image.image) != VK_SUCCESS) {
        PC_ERROR("Failed to create image");
        return image;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Context.getDevice(), image.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Context.getDevice(), &allocInfo, nullptr, &image.memory) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate image memory");
        vkDestroyImage(m_Context.getDevice(), image.image, nullptr);
        image.image = VK_NULL_HANDLE;
        return image;
    }

    vkBindImageMemory(m_Context.getDevice(), image.image, image.memory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_Context.getDevice(), &viewInfo, nullptr, &image.view) != VK_SUCCESS) {
        PC_ERROR("Failed to create image view");
        vkDestroyImage(m_Context.getDevice(), image.image, nullptr);
        vkFreeMemory(m_Context.getDevice(), image.memory, nullptr);
        image.image = VK_NULL_HANDLE;
        image.memory = VK_NULL_HANDLE;
        return image;
    }

    m_Images.push_back(image);
    return image;
}

void VulkanResourceManager::destroyImage(VulkanImage& image) {
    if (image.view != VK_NULL_HANDLE) {
        vkDestroyImageView(m_Context.getDevice(), image.view, nullptr);
        image.view = VK_NULL_HANDLE;
    }
    if (image.image != VK_NULL_HANDLE) {
        vkDestroyImage(m_Context.getDevice(), image.image, nullptr);
        image.image = VK_NULL_HANDLE;
    }
    if (image.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_Context.getDevice(), image.memory, nullptr);
        image.memory = VK_NULL_HANDLE;
    }
}

uint32_t VulkanResourceManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Context.getPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    PC_ERROR("Failed to find suitable memory type");
    return 0;
}

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

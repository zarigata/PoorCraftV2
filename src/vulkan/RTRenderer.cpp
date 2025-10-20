#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/vulkan/RTRenderer.h"
#include "poorcraft/vulkan/VulkanContext.h"
#include "poorcraft/vulkan/VulkanShaderManager.h"
#include "poorcraft/vulkan/VulkanResourceManager.h"
#include "poorcraft/vulkan/RTAccelerationStructure.h"
#include "poorcraft/world/World.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/core/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

namespace PoorCraft {

RTRenderer::RTRenderer(VulkanContext& context, VulkanShaderManager& shaderManager, VulkanResourceManager& resourceManager)
    : m_Context(context)
    , m_ShaderManager(shaderManager)
    , m_ResourceManager(resourceManager) {
    m_AccelStructure = std::make_unique<RTAccelerationStructure>(context);
}

RTRenderer::~RTRenderer() {
    cleanup();
}

bool RTRenderer::initialize() {
    PC_INFO("Initializing RT renderer");

    VkExtent2D extent = m_Context.getSwapchainExtent();
    
    if (!createStorageImage(extent.width, extent.height)) {
        PC_ERROR("Failed to create storage image");
        return false;
    }

    if (!createUniformBuffers()) {
        PC_ERROR("Failed to create uniform buffers");
        return false;
    }

    if (!createDescriptorSets()) {
        PC_ERROR("Failed to create descriptor sets");
        return false;
    }

    if (!createRTPipeline()) {
        PC_ERROR("Failed to create RT pipeline");
        return false;
    }

    if (!createShaderBindingTable()) {
        PC_ERROR("Failed to create shader binding table");
        return false;
    }

    if (!createCompositingResources()) {
        PC_ERROR("Failed to create compositing resources");
        return false;
    }

    m_Initialized = true;
    PC_INFO("RT renderer initialized");
    return true;
}

void RTRenderer::cleanup() {
    if (!m_Initialized) return;

    VkDevice device = m_Context.getDevice();

    if (m_CameraUBO != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, m_CameraUBO, nullptr);
        m_CameraUBO = VK_NULL_HANDLE;
    }

    if (m_CameraUBOMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_CameraUBOMemory, nullptr);
        m_CameraUBOMemory = VK_NULL_HANDLE;
    }

    if (m_StorageImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_StorageImageView, nullptr);
        m_StorageImageView = VK_NULL_HANDLE;
    }

    if (m_StorageImage != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_StorageImage, nullptr);
        m_StorageImage = VK_NULL_HANDLE;
    }

    if (m_StorageImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_StorageImageMemory, nullptr);
        m_StorageImageMemory = VK_NULL_HANDLE;
    }

    if (m_SBTBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, m_SBTBuffer, nullptr);
        m_SBTBuffer = VK_NULL_HANDLE;
    }

    if (m_SBTMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_SBTMemory, nullptr);
        m_SBTMemory = VK_NULL_HANDLE;
    }

    if (m_DescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }

    if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (m_CompositingPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, m_CompositingPipeline, nullptr);
        m_CompositingPipeline = VK_NULL_HANDLE;
    }

    if (m_CompositingPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, m_CompositingPipelineLayout, nullptr);
        m_CompositingPipelineLayout = VK_NULL_HANDLE;
    }

    if (m_CompositingDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_CompositingDescriptorSetLayout, nullptr);
        m_CompositingDescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (m_Sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, m_Sampler, nullptr);
        m_Sampler = VK_NULL_HANDLE;
    }

    if (m_RTPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, m_RTPipeline, nullptr);
        m_RTPipeline = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    if (m_AccelStructure) {
        m_AccelStructure->cleanup();
    }

    m_Initialized = false;
}

bool RTRenderer::createRTPipeline() {
    // Load RT shaders
    VkShaderModule raygenModule = m_ShaderManager.loadShaderModule("shaders/rt/raygen.rgen.spv");
    VkShaderModule missModule = m_ShaderManager.loadShaderModule("shaders/rt/miss.rmiss.spv");
    VkShaderModule closestHitModule = m_ShaderManager.loadShaderModule("shaders/rt/closesthit.rchit.spv");
    
    if (raygenModule == VK_NULL_HANDLE || missModule == VK_NULL_HANDLE || closestHitModule == VK_NULL_HANDLE) {
        PC_ERROR("Failed to load RT shaders");
        return false;
    }

    // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    
    VkPipelineShaderStageCreateInfo raygenStage{};
    raygenStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    raygenStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    raygenStage.module = raygenModule;
    raygenStage.pName = "main";
    shaderStages.push_back(raygenStage);
    
    VkPipelineShaderStageCreateInfo missStage{};
    missStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    missStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    missStage.module = missModule;
    missStage.pName = "main";
    shaderStages.push_back(missStage);
    
    VkPipelineShaderStageCreateInfo closestHitStage{};
    closestHitStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    closestHitStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    closestHitStage.module = closestHitModule;
    closestHitStage.pName = "main";
    shaderStages.push_back(closestHitStage);

    // Shader groups
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    
    // Raygen group
    VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
    raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygenGroup.generalShader = 0;
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(raygenGroup);
    
    // Miss group
    VkRayTracingShaderGroupCreateInfoKHR missGroup{};
    missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missGroup.generalShader = 1;
    missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(missGroup);
    
    // Hit group
    VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
    hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    hitGroup.generalShader = VK_SHADER_UNUSED_KHR;
    hitGroup.closestHitShader = 2;
    hitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    hitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(hitGroup);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(m_Context.getDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        PC_ERROR("Failed to create RT pipeline layout");
        m_ShaderManager.destroyShaderModule(raygenModule);
        m_ShaderManager.destroyShaderModule(missModule);
        m_ShaderManager.destroyShaderModule(closestHitModule);
        return false;
    }

    // Create RT pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();
    pipelineInfo.maxPipelineRayRecursionDepth = 1;
    pipelineInfo.layout = m_PipelineLayout;

    // Get function pointer
    auto vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(
        m_Context.getDevice(), "vkCreateRayTracingPipelinesKHR");
    
    if (!vkCreateRayTracingPipelinesKHR) {
        PC_ERROR("Failed to get vkCreateRayTracingPipelinesKHR function pointer");
        m_ShaderManager.destroyShaderModule(raygenModule);
        m_ShaderManager.destroyShaderModule(missModule);
        m_ShaderManager.destroyShaderModule(closestHitModule);
        return false;
    }

    if (vkCreateRayTracingPipelinesKHR(m_Context.getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_RTPipeline) != VK_SUCCESS) {
        PC_ERROR("Failed to create RT pipeline");
        m_ShaderManager.destroyShaderModule(raygenModule);
        m_ShaderManager.destroyShaderModule(missModule);
        m_ShaderManager.destroyShaderModule(closestHitModule);
        return false;
    }

    // Clean up shader modules
    m_ShaderManager.destroyShaderModule(raygenModule);
    m_ShaderManager.destroyShaderModule(missModule);
    m_ShaderManager.destroyShaderModule(closestHitModule);

    PC_INFO("RT pipeline created successfully");
    return true;
}

bool RTRenderer::createShaderBindingTable() {
    // Get RT properties
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties{};
    rtProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    
    VkPhysicalDeviceProperties2 deviceProperties{};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties.pNext = &rtProperties;
    vkGetPhysicalDeviceProperties2(m_Context.getPhysicalDevice(), &deviceProperties);
    
    uint32_t handleSize = rtProperties.shaderGroupHandleSize;
    uint32_t handleAlignment = rtProperties.shaderGroupHandleAlignment;
    uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
    
    // Calculate aligned handle size
    uint32_t alignedHandleSize = (handleSize + handleAlignment - 1) & ~(handleAlignment - 1);
    
    // We have 3 shader groups: raygen, miss, hit
    uint32_t groupCount = 3;
    
    // Get shader group handles
    std::vector<uint8_t> shaderHandleStorage(groupCount * handleSize);
    
    auto vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(
        m_Context.getDevice(), "vkGetRayTracingShaderGroupHandlesKHR");
    
    if (!vkGetRayTracingShaderGroupHandlesKHR) {
        PC_ERROR("Failed to get vkGetRayTracingShaderGroupHandlesKHR function pointer");
        return false;
    }
    
    if (vkGetRayTracingShaderGroupHandlesKHR(m_Context.getDevice(), m_RTPipeline, 0, groupCount, 
                                             shaderHandleStorage.size(), shaderHandleStorage.data()) != VK_SUCCESS) {
        PC_ERROR("Failed to get RT shader group handles");
        return false;
    }
    
    // Calculate SBT buffer size
    VkDeviceSize raygenSize = baseAlignment;  // Raygen region
    VkDeviceSize missSize = baseAlignment;    // Miss region
    VkDeviceSize hitSize = baseAlignment;     // Hit region
    VkDeviceSize sbtSize = raygenSize + missSize + hitSize;
    
    // Create SBT buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sbtSize;
    bufferInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_Context.getDevice(), &bufferInfo, nullptr, &m_SBTBuffer) != VK_SUCCESS) {
        PC_ERROR("Failed to create SBT buffer");
        return false;
    }
    
    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Context.getDevice(), m_SBTBuffer, &memRequirements);
    
    VkMemoryAllocateFlagsInfo allocFlags{};
    allocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    allocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = &allocFlags;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if (vkAllocateMemory(m_Context.getDevice(), &allocInfo, nullptr, &m_SBTMemory) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate SBT memory");
        return false;
    }
    
    vkBindBufferMemory(m_Context.getDevice(), m_SBTBuffer, m_SBTMemory, 0);
    
    // Map and copy handles
    void* data;
    vkMapMemory(m_Context.getDevice(), m_SBTMemory, 0, sbtSize, 0, &data);
    
    uint8_t* pData = static_cast<uint8_t*>(data);
    
    // Copy raygen handle
    std::memcpy(pData, shaderHandleStorage.data(), handleSize);
    
    // Copy miss handle
    std::memcpy(pData + raygenSize, shaderHandleStorage.data() + handleSize, handleSize);
    
    // Copy hit handle
    std::memcpy(pData + raygenSize + missSize, shaderHandleStorage.data() + 2 * handleSize, handleSize);
    
    vkUnmapMemory(m_Context.getDevice(), m_SBTMemory);
    
    // Get buffer device address
    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = m_SBTBuffer;
    
    auto vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(
        m_Context.getDevice(), "vkGetBufferDeviceAddress");
    
    if (!vkGetBufferDeviceAddress) {
        PC_ERROR("Failed to get vkGetBufferDeviceAddress function pointer");
        return false;
    }
    
    VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(m_Context.getDevice(), &addressInfo);
    
    // Setup SBT regions
    m_RaygenRegion.deviceAddress = sbtAddress;
    m_RaygenRegion.stride = baseAlignment;
    m_RaygenRegion.size = raygenSize;
    
    m_MissRegion.deviceAddress = sbtAddress + raygenSize;
    m_MissRegion.stride = baseAlignment;
    m_MissRegion.size = missSize;
    
    m_HitRegion.deviceAddress = sbtAddress + raygenSize + missSize;
    m_HitRegion.stride = baseAlignment;
    m_HitRegion.size = hitSize;
    
    m_CallableRegion = {}; // Not used
    
    PC_INFO("Shader binding table created");
    return true;
}

uint32_t RTRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
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

bool RTRenderer::createDescriptorSets() {
    // Create descriptor set layout
    // Set 0: Storage image (binding 0), Camera UBO (binding 1)
    
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    
    // Set 0, Binding 0: Storage image
    VkDescriptorSetLayoutBinding storageImageBinding{};
    storageImageBinding.binding = 0;
    storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImageBinding.descriptorCount = 1;
    storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    bindings.push_back(storageImageBinding);
    
    // Set 0, Binding 1: Camera UBO
    VkDescriptorSetLayoutBinding cameraUBOBinding{};
    cameraUBOBinding.binding = 1;
    cameraUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraUBOBinding.descriptorCount = 1;
    cameraUBOBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    bindings.push_back(cameraUBOBinding);
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(m_Context.getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
        PC_ERROR("Failed to create RT descriptor set layout");
        return false;
    }
    
    // Create descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1});
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}); // For compositing
    poolSizes.push_back({VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}); // For future TLAS
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 2; // RT descriptor set + compositing descriptor set
    
    if (vkCreateDescriptorPool(m_Context.getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        PC_ERROR("Failed to create RT descriptor pool");
        return false;
    }
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayout;
    
    if (vkAllocateDescriptorSets(m_Context.getDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate RT descriptor set");
        return false;
    }
    
    // Update descriptor set
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    
    // Storage image
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = m_StorageImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet imageWrite{};
    imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imageWrite.dstSet = m_DescriptorSet;
    imageWrite.dstBinding = 0;
    imageWrite.dstArrayElement = 0;
    imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageWrite.descriptorCount = 1;
    imageWrite.pImageInfo = &imageInfo;
    descriptorWrites.push_back(imageWrite);
    
    // Camera UBO
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_CameraUBO;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;
    
    VkWriteDescriptorSet bufferWrite{};
    bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    bufferWrite.dstSet = m_DescriptorSet;
    bufferWrite.dstBinding = 1;
    bufferWrite.dstArrayElement = 0;
    bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferWrite.descriptorCount = 1;
    bufferWrite.pBufferInfo = &bufferInfo;
    descriptorWrites.push_back(bufferWrite);
    
    vkUpdateDescriptorSets(m_Context.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), 
                          descriptorWrites.data(), 0, nullptr);
    
    PC_INFO("RT descriptor sets created");
    return true;
}

bool RTRenderer::createStorageImage(uint32_t width, uint32_t height) {
    // Create storage image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if (vkCreateImage(m_Context.getDevice(), &imageInfo, nullptr, &m_StorageImage) != VK_SUCCESS) {
        PC_ERROR("Failed to create storage image");
        return false;
    }
    
    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Context.getDevice(), m_StorageImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(m_Context.getDevice(), &allocInfo, nullptr, &m_StorageImageMemory) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate storage image memory");
        return false;
    }
    
    vkBindImageMemory(m_Context.getDevice(), m_StorageImage, m_StorageImageMemory, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_StorageImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(m_Context.getDevice(), &viewInfo, nullptr, &m_StorageImageView) != VK_SUCCESS) {
        PC_ERROR("Failed to create storage image view");
        return false;
    }
    
    // Transition image layout to GENERAL
    VkCommandBuffer commandBuffer = m_Context.getCommandBuffer(0);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_StorageImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(m_Context.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_Context.getGraphicsQueue());
    
    PC_INFO("Storage image created");
    return true;
}

bool RTRenderer::createUniformBuffers() {
    // CameraUBO structure size (matching shader layout with padding)
    VkDeviceSize bufferSize = sizeof(glm::mat4) * 4 +  // view, projection, invView, invProjection
                              sizeof(glm::vec4) * 5 +  // position+pad, sunDirection+pad, sunColor+pad, skyTopColor+pad, skyHorizonColor+ambientStrength
                              sizeof(float);            // timeOfDay
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_Context.getDevice(), &bufferInfo, nullptr, &m_CameraUBO) != VK_SUCCESS) {
        PC_ERROR("Failed to create camera UBO");
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Context.getDevice(), m_CameraUBO, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if (vkAllocateMemory(m_Context.getDevice(), &allocInfo, nullptr, &m_CameraUBOMemory) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate camera UBO memory");
        return false;
    }
    
    vkBindBufferMemory(m_Context.getDevice(), m_CameraUBO, m_CameraUBOMemory, 0);
    
    PC_INFO("Uniform buffers created");
    return true;
}

void RTRenderer::traceRays(VkCommandBuffer commandBuffer, World& world, Camera& camera, uint32_t width, uint32_t height) {
    (void)world;
    
    // Update camera UBO
    struct CameraUBO {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 invView;
        glm::mat4 invProjection;
        glm::vec3 position;
        float _pad0;
        glm::vec3 sunDirection;
        float _pad1;
        glm::vec3 sunColor;
        float _pad2;
        glm::vec3 skyTopColor;
        float _pad3;
        glm::vec3 skyHorizonColor;
        float ambientStrength;
        float timeOfDay;
    } ubo;
    
    ubo.view = camera.getViewMatrix();
    ubo.projection = camera.getProjectionMatrix();
    ubo.invView = glm::inverse(ubo.view);
    ubo.invProjection = glm::inverse(ubo.projection);
    ubo.position = camera.getPosition();
    ubo.sunDirection = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
    ubo.sunColor = glm::vec3(1.0f, 0.95f, 0.8f);
    ubo.skyTopColor = glm::vec3(0.5f, 0.7f, 1.0f);
    ubo.skyHorizonColor = glm::vec3(0.8f, 0.9f, 1.0f);
    ubo.ambientStrength = 0.3f;
    ubo.timeOfDay = 0.5f;
    
    void* data;
    vkMapMemory(m_Context.getDevice(), m_CameraUBOMemory, 0, sizeof(ubo), 0, &data);
    std::memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_Context.getDevice(), m_CameraUBOMemory);
    
    // Bind RT pipeline
    auto vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(m_Context.getDevice(), "vkCmdBindPipeline");
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTPipeline);
    
    // Bind descriptor sets
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_PipelineLayout, 
                           0, 1, &m_DescriptorSet, 0, nullptr);
    
    // Trace rays
    auto vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(m_Context.getDevice(), "vkCmdTraceRaysKHR");
    
    if (vkCmdTraceRaysKHR) {
        vkCmdTraceRaysKHR(commandBuffer, &m_RaygenRegion, &m_MissRegion, &m_HitRegion, &m_CallableRegion, width, height, 1);
    }
}

bool RTRenderer::createCompositingResources() {
    VkDevice device = m_Context.getDevice();
    
    // Create sampler for sampling the RT storage image
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
        PC_ERROR("Failed to create sampler for compositing");
        return false;
    }
    
    // Create descriptor set layout for compositing (sampled image)
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerBinding;
    
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_CompositingDescriptorSetLayout) != VK_SUCCESS) {
        PC_ERROR("Failed to create compositing descriptor set layout");
        return false;
    }
    
    // Allocate descriptor set from existing pool
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_CompositingDescriptorSetLayout;
    
    if (vkAllocateDescriptorSets(device, &allocInfo, &m_CompositingDescriptorSet) != VK_SUCCESS) {
        PC_ERROR("Failed to allocate compositing descriptor set");
        return false;
    }
    
    // Update descriptor set with storage image view and sampler
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_StorageImageView;
    imageInfo.sampler = m_Sampler;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_CompositingDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    
    PC_INFO("Compositing resources created");
    return true;
}

void RTRenderer::composite(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t width, uint32_t height) {
    // Transition storage image from GENERAL to SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_StorageImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {width, height};
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Create compositing pipeline if not already created
    if (m_CompositingPipeline == VK_NULL_HANDLE) {
        // Load shaders
        VkShaderModule vertShader = m_ShaderManager.loadShaderModule("shaders/basic/fullscreen.vert.spv");
        VkShaderModule fragShader = m_ShaderManager.loadShaderModule("shaders/basic/fullscreen.frag.spv");
        
        if (vertShader == VK_NULL_HANDLE || fragShader == VK_NULL_HANDLE) {
            PC_ERROR("Failed to load compositing shaders");
            vkCmdEndRenderPass(commandBuffer);
            return;
        }
        
        // Shader stages
        VkPipelineShaderStageCreateInfo vertStageInfo{};
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = vertShader;
        vertStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo fragStageInfo{};
        fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageInfo.module = fragShader;
        fragStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};
        
        // Vertex input (none - fullscreen triangle)
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        
        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        
        // Viewport and scissor (dynamic)
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {width, height};
        
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        
        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        
        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        
        // Color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        
        // Pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_CompositingDescriptorSetLayout;
        
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_CompositingPipelineLayout) != VK_SUCCESS) {
            PC_ERROR("Failed to create compositing pipeline layout");
            m_ShaderManager.destroyShaderModule(vertShader);
            m_ShaderManager.destroyShaderModule(fragShader);
            vkCmdEndRenderPass(commandBuffer);
            return;
        }
        
        // Create graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = m_CompositingPipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_CompositingPipeline) != VK_SUCCESS) {
            PC_ERROR("Failed to create compositing pipeline");
            m_ShaderManager.destroyShaderModule(vertShader);
            m_ShaderManager.destroyShaderModule(fragShader);
            vkCmdEndRenderPass(commandBuffer);
            return;
        }
        
        m_ShaderManager.destroyShaderModule(vertShader);
        m_ShaderManager.destroyShaderModule(fragShader);
        
        PC_INFO("Compositing pipeline created");
    }
    
    // Bind pipeline and draw fullscreen triangle
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CompositingPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CompositingPipelineLayout, 0, 1, &m_CompositingDescriptorSet, 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    
    // Don't end render pass here - ImGui will render in the same pass
}

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

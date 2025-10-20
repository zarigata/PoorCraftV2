#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

namespace PoorCraft {

// Forward declarations
class VulkanContext;

/**
 * Manages Vulkan shader modules
 * Loads SPIR-V files and creates VkShaderModule objects
 */
class VulkanShaderManager {
public:
    explicit VulkanShaderManager(VulkanContext& context);
    ~VulkanShaderManager();

    // Non-copyable
    VulkanShaderManager(const VulkanShaderManager&) = delete;
    VulkanShaderManager& operator=(const VulkanShaderManager&) = delete;

    /**
     * Load a SPIR-V shader file and create a shader module
     * @param filepath Path to .spv file
     * @return VkShaderModule handle (VK_NULL_HANDLE on failure)
     */
    VkShaderModule loadShaderModule(const std::string& filepath);

    /**
     * Destroy a shader module
     */
    void destroyShaderModule(VkShaderModule module);

    /**
     * Cleanup all shader modules
     */
    void cleanup();

private:
    std::vector<char> readFile(const std::string& filepath);

    VulkanContext& m_Context;
    std::vector<VkShaderModule> m_ShaderModules;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

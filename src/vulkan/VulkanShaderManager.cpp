#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/vulkan/VulkanShaderManager.h"
#include "poorcraft/vulkan/VulkanContext.h"
#include "poorcraft/core/Logger.h"
#include <fstream>

namespace PoorCraft {

VulkanShaderManager::VulkanShaderManager(VulkanContext& context)
    : m_Context(context) {
}

VulkanShaderManager::~VulkanShaderManager() {
    cleanup();
}

VkShaderModule VulkanShaderManager::loadShaderModule(const std::string& filepath) {
    std::vector<char> code = readFile(filepath);
    
    if (code.empty()) {
        PC_ERROR("Failed to read shader file: {}", filepath);
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Context.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        PC_ERROR("Failed to create shader module from: {}", filepath);
        return VK_NULL_HANDLE;
    }

    m_ShaderModules.push_back(shaderModule);
    PC_INFO("Loaded shader module: {}", filepath);
    return shaderModule;
}

void VulkanShaderManager::destroyShaderModule(VkShaderModule module) {
    if (module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_Context.getDevice(), module, nullptr);
        
        // Remove from tracking
        auto it = std::find(m_ShaderModules.begin(), m_ShaderModules.end(), module);
        if (it != m_ShaderModules.end()) {
            m_ShaderModules.erase(it);
        }
    }
}

void VulkanShaderManager::cleanup() {
    for (auto module : m_ShaderModules) {
        vkDestroyShaderModule(m_Context.getDevice(), module, nullptr);
    }
    m_ShaderModules.clear();
}

std::vector<char> VulkanShaderManager::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        PC_ERROR("Failed to open file: {}", filepath);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

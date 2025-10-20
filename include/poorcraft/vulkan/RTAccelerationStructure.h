#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <vector>

namespace PoorCraft {

// Forward declarations
class VulkanContext;

/**
 * Ray tracing acceleration structure builder
 * Manages BLAS (bottom-level) and TLAS (top-level) acceleration structures
 */
class RTAccelerationStructure {
public:
    explicit RTAccelerationStructure(VulkanContext& context);
    ~RTAccelerationStructure();

    // Non-copyable
    RTAccelerationStructure(const RTAccelerationStructure&) = delete;
    RTAccelerationStructure& operator=(const RTAccelerationStructure&) = delete;

    /**
     * Build a bottom-level acceleration structure (BLAS) from geometry
     * @param vertexBuffer Vertex buffer
     * @param indexBuffer Index buffer
     * @param vertexCount Number of vertices
     * @param indexCount Number of indices
     * @return BLAS handle
     */
    VkAccelerationStructureKHR buildBLAS(
        VkBuffer vertexBuffer,
        VkBuffer indexBuffer,
        uint32_t vertexCount,
        uint32_t indexCount
    );

    /**
     * Build a top-level acceleration structure (TLAS) from instances
     * @param instances Array of BLAS instances
     * @return TLAS handle
     */
    VkAccelerationStructureKHR buildTLAS(
        const std::vector<VkAccelerationStructureInstanceKHR>& instances
    );

    /**
     * Cleanup all acceleration structures
     */
    void cleanup();

private:
    VulkanContext& m_Context;
    std::vector<VkAccelerationStructureKHR> m_AccelerationStructures;
    std::vector<VkBuffer> m_AccelBuffers;
    std::vector<VkDeviceMemory> m_AccelMemory;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

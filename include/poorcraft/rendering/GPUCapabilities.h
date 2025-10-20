#pragma once

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>

namespace PoorCraft {

enum class GPUVendor {
    NVIDIA,
    AMD,
    INTEL,
    UNKNOWN
};

struct GPUCapabilitiesData {
    GPUVendor vendor = GPUVendor::UNKNOWN;
    std::string vendorString;
    std::string rendererString;
    std::string versionString;
    std::string glslVersionString;

    int glVersionMajor = 0;
    int glVersionMinor = 0;
    int glslVersionMajor = 0;
    int glslVersionMinor = 0;

    int maxTextureSize = 0;
    int maxTextureUnits = 0;
    int maxVertexAttributes = 0;
    int maxUniformBufferBindings = 0;
    int maxColorAttachments = 0;
    int maxSamples = 0;

    float maxAnisotropy = 0.0f;

    bool supportsComputeShaders = false;
    bool supportsGeometryShaders = false;
    bool supportsTessellationShaders = false;
    bool supportsBindlessTextures = false;
    bool supportsMultiDrawIndirect = false;
    bool supportsDebugOutput = false;

    std::size_t totalVRAMMB = 0;
    std::size_t availableVRAMMB = 0;

    // Vulkan capabilities
    bool vulkanSupported = false;
    std::string vulkanVersionString;
    bool supportsRayTracing = false;
    uint32_t shaderGroupHandleSize = 0;
    uint32_t maxRayRecursionDepth = 0;
};

class GPUCapabilities {
public:
    static GPUCapabilities& getInstance();

    bool query();

    const GPUCapabilitiesData& getCapabilities() const;

    GPUVendor getVendor() const;
    const std::string& getVendorString() const;
    const std::string& getRendererString() const;
    const std::string& getVersionString() const;
    const std::string& getGLSLVersionString() const;

    int getMaxTextureSize() const;
    int getMaxTextureUnits() const;
    int getMaxVertexAttributes() const;
    int getMaxUniformBufferBindings() const;
    int getMaxColorAttachments() const;
    int getMaxSamples() const;
    float getMaxAnisotropy() const;

    bool supportsExtension(const std::string& name) const;
    bool supportsCompute() const;
    bool supportsGeometry() const;
    bool supportsTessellation() const;
    bool supportsMultiDrawIndirectRendering() const;
    bool supportsDebugOutputMessages() const;

    std::size_t getTotalVRAMMB() const;
    std::size_t getAvailableVRAMMB() const;

    bool isVendor(GPUVendor vendor) const;

    bool requiresWorkaround(const std::string& issueKey) const;

    void printCapabilities() const;

    // Vulkan capabilities
    bool supportsVulkan() const;
    bool supportsRayTracingPipeline() const;
    const std::string& getVulkanVersion() const;
    uint32_t getShaderGroupHandleSize() const;
    uint32_t getMaxRayRecursionDepth() const;

private:
    GPUCapabilities() = default;

    GPUVendor parseVendor(const std::string& renderer) const;
    void queryVRAM();
    void queryVulkan();

private:
    GPUCapabilitiesData m_Data;
    std::set<std::string> m_Extensions;
};

} // namespace PoorCraft

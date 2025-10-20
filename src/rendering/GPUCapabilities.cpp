#include "poorcraft/rendering/GPUCapabilities.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstring>
#include <vector>

#include <glad/glad.h>

#ifdef POORCRAFT_VULKAN_SUPPORT
#include <vulkan/vulkan.h>
#endif

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

namespace {
GPUVendor detectVendor(const std::string& renderer) {
    std::string lowerRenderer;
    lowerRenderer.reserve(renderer.size());
    std::transform(renderer.begin(), renderer.end(), std::back_inserter(lowerRenderer), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (lowerRenderer.find("nvidia") != std::string::npos || lowerRenderer.find("geforce") != std::string::npos) {
        return GPUVendor::NVIDIA;
    }
    if (lowerRenderer.find("amd") != std::string::npos || lowerRenderer.find("radeon") != std::string::npos) {
        return GPUVendor::AMD;
    }
    if (lowerRenderer.find("intel") != std::string::npos) {
        return GPUVendor::INTEL;
    }
    return GPUVendor::UNKNOWN;
}

void parseVersion(const std::string& versionStr, int& major, int& minor) {
    major = 0;
    minor = 0;
    std::istringstream iss(versionStr);
    iss >> major;
    if (iss.peek() == '.') {
        iss.ignore();
        iss >> minor;
    }
}

bool hasExtension(const std::set<std::string>& extensions, const std::string& name) {
    return extensions.find(name) != extensions.end();
}

} // namespace

GPUCapabilities& GPUCapabilities::getInstance() {
    static GPUCapabilities instance;
    return instance;
}

bool GPUCapabilities::query() {
    // Query Vulkan first (doesn't require OpenGL context)
    queryVulkan();
    
    m_Extensions.clear();

    const GLubyte* vendorStr = glGetString(GL_VENDOR);
    const GLubyte* rendererStr = glGetString(GL_RENDERER);
    const GLubyte* versionStr = glGetString(GL_VERSION);
    const GLubyte* glslStr = glGetString(GL_SHADING_LANGUAGE_VERSION);

    if (!vendorStr || !rendererStr || !versionStr) {
        PC_WARN("OpenGL context not available, skipping OpenGL capability query");
        PC_INFO("Vulkan capabilities queried successfully");
        return true; // Return true if Vulkan was queried successfully
    }

    m_Data.vendorString = reinterpret_cast<const char*>(vendorStr);
    m_Data.rendererString = reinterpret_cast<const char*>(rendererStr);
    m_Data.versionString = reinterpret_cast<const char*>(versionStr);
    m_Data.glslVersionString = glslStr ? reinterpret_cast<const char*>(glslStr) : "";
    m_Data.vendor = detectVendor(m_Data.rendererString);

    parseVersion(m_Data.versionString, m_Data.glVersionMajor, m_Data.glVersionMinor);
    parseVersion(m_Data.glslVersionString, m_Data.glslVersionMajor, m_Data.glslVersionMinor);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_Data.maxTextureSize);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_Data.maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_Data.maxVertexAttributes);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &m_Data.maxUniformBufferBindings);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_Data.maxColorAttachments);
    glGetIntegerv(GL_MAX_SAMPLES, &m_Data.maxSamples);

    if (GLAD_GL_EXT_texture_filter_anisotropic) {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_Data.maxAnisotropy);
    } else {
        m_Data.maxAnisotropy = 1.0f;
    }

    GLint extensionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
    for (GLint i = 0; i < extensionCount; ++i) {
        const char* extensionName = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
        if (extensionName) {
            m_Extensions.insert(extensionName);
        }
    }

    m_Data.supportsComputeShaders = (m_Data.glVersionMajor > 4) || (m_Data.glVersionMajor == 4 && m_Data.glVersionMinor >= 3) || hasExtension(m_Extensions, "GL_ARB_compute_shader");
    m_Data.supportsGeometryShaders = (m_Data.glVersionMajor > 3) || (m_Data.glVersionMajor == 3 && m_Data.glVersionMinor >= 2) || hasExtension(m_Extensions, "GL_EXT_geometry_shader4");
    m_Data.supportsTessellationShaders = (m_Data.glVersionMajor > 4) || (m_Data.glVersionMajor == 4 && m_Data.glVersionMinor >= 0) || hasExtension(m_Extensions, "GL_ARB_tessellation_shader");
    m_Data.supportsBindlessTextures = hasExtension(m_Extensions, "GL_NV_bindless_texture") || hasExtension(m_Extensions, "GL_ARB_bindless_texture");
    m_Data.supportsMultiDrawIndirect = (m_Data.glVersionMajor > 4) || (m_Data.glVersionMajor == 4 && m_Data.glVersionMinor >= 3) || hasExtension(m_Extensions, "GL_ARB_multi_draw_indirect");
    m_Data.supportsDebugOutput = hasExtension(m_Extensions, "GL_KHR_debug");

    queryVRAM();

    printCapabilities();

    return true;
}

const GPUCapabilitiesData& GPUCapabilities::getCapabilities() const {
    return m_Data;
}

GPUVendor GPUCapabilities::getVendor() const {
    return m_Data.vendor;
}

const std::string& GPUCapabilities::getVendorString() const {
    return m_Data.vendorString;
}

const std::string& GPUCapabilities::getRendererString() const {
    return m_Data.rendererString;
}

const std::string& GPUCapabilities::getVersionString() const {
    return m_Data.versionString;
}

const std::string& GPUCapabilities::getGLSLVersionString() const {
    return m_Data.glslVersionString;
}

int GPUCapabilities::getMaxTextureSize() const {
    return m_Data.maxTextureSize;
}

int GPUCapabilities::getMaxTextureUnits() const {
    return m_Data.maxTextureUnits;
}

int GPUCapabilities::getMaxVertexAttributes() const {
    return m_Data.maxVertexAttributes;
}

int GPUCapabilities::getMaxUniformBufferBindings() const {
    return m_Data.maxUniformBufferBindings;
}

int GPUCapabilities::getMaxColorAttachments() const {
    return m_Data.maxColorAttachments;
}

int GPUCapabilities::getMaxSamples() const {
    return m_Data.maxSamples;
}

float GPUCapabilities::getMaxAnisotropy() const {
    return m_Data.maxAnisotropy;
}

bool GPUCapabilities::supportsExtension(const std::string& name) const {
    return hasExtension(m_Extensions, name);
}

bool GPUCapabilities::supportsCompute() const {
    return m_Data.supportsComputeShaders;
}

bool GPUCapabilities::supportsGeometry() const {
    return m_Data.supportsGeometryShaders;
}

bool GPUCapabilities::supportsTessellation() const {
    return m_Data.supportsTessellationShaders;
}

bool GPUCapabilities::supportsMultiDrawIndirectRendering() const {
    return m_Data.supportsMultiDrawIndirect;
}

bool GPUCapabilities::supportsDebugOutputMessages() const {
    return m_Data.supportsDebugOutput;
}

std::size_t GPUCapabilities::getTotalVRAMMB() const {
    return m_Data.totalVRAMMB;
}

std::size_t GPUCapabilities::getAvailableVRAMMB() const {
    return m_Data.availableVRAMMB;
}

bool GPUCapabilities::isVendor(GPUVendor vendor) const {
    return m_Data.vendor == vendor;
}

bool GPUCapabilities::requiresWorkaround(const std::string& issueKey) const {
    if (issueKey == "intel_depth_clip") {
        return m_Data.vendor == GPUVendor::INTEL && m_Data.glVersionMajor < 4;
    }
    return false;
}

void GPUCapabilities::printCapabilities() const {
    PC_INFO("=== GPU Capabilities ===");
    PC_INFOF("Vendor: %s", m_Data.vendorString.c_str());
    PC_INFOF("Renderer: %s", m_Data.rendererString.c_str());
    PC_INFOF("OpenGL Version: %s", m_Data.versionString.c_str());
    PC_INFOF("GLSL Version: %s", m_Data.glslVersionString.c_str());
    PC_INFOF("Max Texture Size: %d", m_Data.maxTextureSize);
    PC_INFOF("Max Texture Units: %d", m_Data.maxTextureUnits);
    PC_INFOF("Max Vertex Attributes: %d", m_Data.maxVertexAttributes);
    PC_INFOF("Max Uniform Buffer Bindings: %d", m_Data.maxUniformBufferBindings);
    PC_INFOF("Max Color Attachments: %d", m_Data.maxColorAttachments);
    PC_INFOF("Max Samples: %d", m_Data.maxSamples);
    PC_INFOF("Max Anisotropy: %.2f", m_Data.maxAnisotropy);
    PC_INFOF("Total VRAM: %zu MB", m_Data.totalVRAMMB);
    PC_INFOF("Available VRAM: %zu MB", m_Data.availableVRAMMB);
    PC_INFO("========================");
}

GPUVendor GPUCapabilities::parseVendor(const std::string& renderer) const {
    return detectVendor(renderer);
}

void GPUCapabilities::queryVRAM() {
    m_Data.totalVRAMMB = 0;
    m_Data.availableVRAMMB = 0;

    if (hasExtension(m_Extensions, "GL_NVX_gpu_memory_info")) {
#ifdef GL_NVX_gpu_memory_info
        if (hasExtension(m_Extensions, "GL_NVX_gpu_memory_info")) {
            GLint totalMemoryKB = 0;
            GLint availableMemoryKB = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKB);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemoryKB);
            m_Data.totalVRAMMB = static_cast<std::size_t>(totalMemoryKB) / 1024;
            m_Data.availableVRAMMB = static_cast<std::size_t>(availableMemoryKB) / 1024;
        }
#else
        PC_WARN("GL_NVX_gpu_memory_info extension detected but enums not available in GLAD headers");
#endif
    } else if (hasExtension(m_Extensions, "GL_ATI_meminfo")) {
#ifdef GL_ATI_meminfo
        if (hasExtension(m_Extensions, "GL_ATI_meminfo")) {
            GLint vboMemory[4] = {0};
            glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, vboMemory);
            m_Data.totalVRAMMB = static_cast<std::size_t>(vboMemory[0]) / 1024;
            m_Data.availableVRAMMB = static_cast<std::size_t>(vboMemory[1]) / 1024;
        }
#else
        PC_WARN("GL_ATI_meminfo extension detected but enums not available in GLAD headers");
#endif
    }
}

void GPUCapabilities::queryVulkan() {
#ifdef POORCRAFT_VULKAN_SUPPORT
    PC_INFO("Querying Vulkan capabilities...");
    
    // Create a temporary Vulkan instance for capability detection
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "PoorCraft";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "PoorCraft Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    VkInstance tempInstance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &tempInstance);
    
    if (result != VK_SUCCESS) {
        PC_WARN("Failed to create temporary Vulkan instance for capability query");
        m_Data.vulkanSupported = false;
        m_Data.vulkanVersionString = "Not available";
        m_Data.supportsRayTracing = false;
        return;
    }

    // Query instance version
    uint32_t instanceVersion = 0;
    vkEnumerateInstanceVersion(&instanceVersion);
    uint32_t major = VK_VERSION_MAJOR(instanceVersion);
    uint32_t minor = VK_VERSION_MINOR(instanceVersion);
    uint32_t patch = VK_VERSION_PATCH(instanceVersion);
    m_Data.vulkanVersionString = std::to_string(major) + "." + 
                                   std::to_string(minor) + "." + 
                                   std::to_string(patch);

    // Enumerate physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(tempInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        PC_WARN("No Vulkan-capable GPUs found");
        m_Data.vulkanSupported = false;
        m_Data.supportsRayTracing = false;
        vkDestroyInstance(tempInstance, nullptr);
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(tempInstance, &deviceCount, devices.data());

    // Select best device (prefer discrete GPU)
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestScore = -1;
    
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        
        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        
        if (score > bestScore) {
            bestScore = score;
            bestDevice = device;
        }
    }

    if (bestDevice == VK_NULL_HANDLE) {
        bestDevice = devices[0];
    }

    // Query RT extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(bestDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(bestDevice, nullptr, &extensionCount, availableExtensions.data());

    bool hasRTPipeline = false;
    bool hasAccelStruct = false;
    bool hasBufferDeviceAddress = false;
    bool hasDeferredHostOps = false;

    for (const auto& ext : availableExtensions) {
        if (strcmp(ext.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
            hasRTPipeline = true;
        }
        if (strcmp(ext.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
            hasAccelStruct = true;
        }
        if (strcmp(ext.extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0) {
            hasBufferDeviceAddress = true;
        }
        if (strcmp(ext.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
            hasDeferredHostOps = true;
        }
    }

    bool rtExtensionsAvailable = hasRTPipeline && hasAccelStruct && hasBufferDeviceAddress && hasDeferredHostOps;

    // Query RT features
    bool rtFeaturesSupported = false;
    if (rtExtensionsAvailable) {
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
        rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
        accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelStructFeatures.pNext = &rtPipelineFeatures;

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        bufferDeviceAddressFeatures.pNext = &accelStructFeatures;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &bufferDeviceAddressFeatures;

        vkGetPhysicalDeviceFeatures2(bestDevice, &deviceFeatures2);

        rtFeaturesSupported = rtPipelineFeatures.rayTracingPipeline && 
                              accelStructFeatures.accelerationStructure &&
                              bufferDeviceAddressFeatures.bufferDeviceAddress;

        // Query RT properties
        if (rtFeaturesSupported) {
            VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtPipelineProps{};
            rtPipelineProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

            VkPhysicalDeviceProperties2 deviceProps2{};
            deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProps2.pNext = &rtPipelineProps;

            vkGetPhysicalDeviceProperties2(bestDevice, &deviceProps2);

            m_Data.shaderGroupHandleSize = rtPipelineProps.shaderGroupHandleSize;
            m_Data.maxRayRecursionDepth = rtPipelineProps.maxRayRecursionDepth;
        }
    }

    m_Data.vulkanSupported = true;
    m_Data.supportsRayTracing = rtFeaturesSupported;

    PC_INFO("Vulkan support: Available (version {})", m_Data.vulkanVersionString);
    PC_INFO("Ray tracing support: {}", m_Data.supportsRayTracing ? "Yes" : "No");
    if (m_Data.supportsRayTracing) {
        PC_INFO("  Shader group handle size: {}", m_Data.shaderGroupHandleSize);
        PC_INFO("  Max ray recursion depth: {}", m_Data.maxRayRecursionDepth);
    }

    // Cleanup
    vkDestroyInstance(tempInstance, nullptr);
#else
    m_Data.vulkanSupported = false;
    m_Data.vulkanVersionString = "Not compiled";
    m_Data.supportsRayTracing = false;
    PC_INFO("Vulkan support: Not compiled");
#endif
}

bool GPUCapabilities::supportsVulkan() const {
    return m_Data.vulkanSupported;
}

bool GPUCapabilities::supportsRayTracingPipeline() const {
    return m_Data.supportsRayTracing;
}

const std::string& GPUCapabilities::getVulkanVersion() const {
    return m_Data.vulkanVersionString;
}

uint32_t GPUCapabilities::getShaderGroupHandleSize() const {
    return m_Data.shaderGroupHandleSize;
}

uint32_t GPUCapabilities::getMaxRayRecursionDepth() const {
    return m_Data.maxRayRecursionDepth;
}

} // namespace PoorCraft

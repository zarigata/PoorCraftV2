#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/vulkan/VulkanBackend.h"
#include "poorcraft/vulkan/VulkanContext.h"
#include "poorcraft/vulkan/VulkanShaderManager.h"
#include "poorcraft/vulkan/VulkanResourceManager.h"
#include "poorcraft/vulkan/VulkanRasterRenderer.h"
#include "poorcraft/vulkan/RTRenderer.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/core/Logger.h"
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace PoorCraft {

VulkanBackend::VulkanBackend(Window& window, bool enableRayTracing)
    : m_Window(window)
    , m_RayTracingEnabled(enableRayTracing) {
}

VulkanBackend::~VulkanBackend() {
    if (m_Initialized) {
        shutdown();
    }
}

bool VulkanBackend::initialize() {
    PC_INFO("Initializing Vulkan backend (RT: {})", m_RayTracingEnabled ? "enabled" : "disabled");

    // Create Vulkan context
    m_Context = std::make_unique<VulkanContext>(m_Window);
    
    bool enableValidation = true; // Enable validation in debug builds
    #ifdef NDEBUG
    enableValidation = false;
    #endif

    if (!m_Context->initialize(enableValidation, m_RayTracingEnabled)) {
        PC_ERROR("Failed to initialize Vulkan context");
        return false;
    }

    // Check if RT was requested but not supported
    if (m_RayTracingEnabled && !m_Context->supportsRayTracing()) {
        PC_WARN("Ray tracing requested but not supported by device. Falling back to raster");
        m_RayTracingEnabled = false;
    }

    // Create shader manager
    m_ShaderManager = std::make_unique<VulkanShaderManager>(*m_Context);

    // Create resource manager
    m_ResourceManager = std::make_unique<VulkanResourceManager>(*m_Context);
    if (!m_ResourceManager->initialize()) {
        PC_ERROR("Failed to initialize resource manager");
        return false;
    }

    // Create raster renderer (always needed for UI and fallback)
    m_RasterRenderer = std::make_unique<VulkanRasterRenderer>(*m_Context, *m_ShaderManager);
    if (!m_RasterRenderer->initialize()) {
        PC_ERROR("Failed to initialize raster renderer");
        return false;
    }

    // Create RT renderer if enabled
    if (m_RayTracingEnabled) {
        m_RTRenderer = std::make_unique<RTRenderer>(*m_Context, *m_ShaderManager, *m_ResourceManager);
        if (!m_RTRenderer->initialize()) {
            PC_WARN("Failed to initialize RT renderer, falling back to raster");
            m_RayTracingEnabled = false;
            m_RTRenderer.reset();
        }
    }

    // Initialize ImGui Vulkan backend
    if (!initializeImGui()) {
        PC_ERROR("Failed to initialize ImGui Vulkan backend");
        return false;
    }

    m_Initialized = true;
    PC_INFO("Vulkan backend initialized successfully");
    return true;
}

void VulkanBackend::shutdown() {
    PC_INFO("Shutting down Vulkan backend");
    
    // Shutdown ImGui
    if (m_Context) {
        vkDeviceWaitIdle(m_Context->getDevice());
        ImGui_ImplVulkan_Shutdown();
        
        if (m_ImGuiDescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_Context->getDevice(), m_ImGuiDescriptorPool, nullptr);
            m_ImGuiDescriptorPool = VK_NULL_HANDLE;
        }
    }
    
    if (m_RTRenderer) {
        m_RTRenderer->cleanup();
        m_RTRenderer.reset();
    }
    
    if (m_RasterRenderer) {
        m_RasterRenderer->cleanup();
        m_RasterRenderer.reset();
    }
    
    if (m_ShaderManager) {
        m_ShaderManager->cleanup();
        m_ShaderManager.reset();
    }
    
    if (m_ResourceManager) {
        m_ResourceManager->shutdown();
        m_ResourceManager.reset();
    }
    
    if (m_Context) {
        m_Context->shutdown();
        m_Context.reset();
    }
    
    m_Initialized = false;
}

void VulkanBackend::beginFrame() {
    uint32_t currentFrame = m_Context->getCurrentFrame();
    
    // Wait for the fence before reusing frame resources
    VkFence inFlightFence = m_Context->getInFlightFence(currentFrame);
    vkWaitForFences(m_Context->getDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    
    VkResult result = m_Context->acquireNextImage(m_CurrentImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleSwapchainRecreation();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        PC_ERROR("Failed to acquire swapchain image");
        return;
    }
    
    // Reset the fence after successful image acquisition
    vkResetFences(m_Context->getDevice(), 1, &inFlightFence);

    // Begin command buffer recording
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(currentFrame);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        PC_ERROR("Failed to begin recording command buffer");
    }

    m_Stats = RenderStats(); // Reset stats
}

void VulkanBackend::endFrame() {
    uint32_t currentFrame = m_Context->getCurrentFrame();
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(currentFrame);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        PC_ERROR("Failed to record command buffer");
        return;
    }

    // Submit command buffer with proper synchronization
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait on image-available semaphore at COLOR_ATTACHMENT_OUTPUT stage
    VkSemaphore imageAvailableSemaphore = m_Context->getImageAvailableSemaphore(currentFrame);
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    
    // Submit the command buffer
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Signal render-finished semaphore when done
    VkSemaphore renderFinishedSemaphore = m_Context->getRenderFinishedSemaphore(currentFrame);
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

    // Submit with in-flight fence
    VkFence inFlightFence = m_Context->getInFlightFence(currentFrame);
    if (vkQueueSubmit(m_Context->getGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        PC_ERROR("Failed to submit draw command buffer");
        return;
    }

    // Present, waiting on render-finished semaphore
    VkResult result = m_Context->presentImage(m_CurrentImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        handleSwapchainRecreation();
    } else if (result != VK_SUCCESS) {
        PC_ERROR("Failed to present swapchain image");
    }
}

void VulkanBackend::clear() {
    // Clear will be handled in render pass
}

void VulkanBackend::setClearColor(float r, float g, float b, float a) {
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
}

void VulkanBackend::setViewport(int x, int y, int width, int height) {
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(m_Context->getCurrentFrame());
    
    VkViewport viewport{};
    viewport.x = static_cast<float>(x);
    viewport.y = static_cast<float>(y);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {x, y};
    scissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanBackend::renderWorld(World& world, Camera& camera, float deltaTime) {
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(m_Context->getCurrentFrame());
    
    if (m_RayTracingEnabled && m_RTRenderer) {
        // Use ray tracing path
        VkExtent2D extent = m_Context->getSwapchainExtent();
        m_RTRenderer->traceRays(commandBuffer, world, camera, extent.width, extent.height);
    } else if (m_RasterRenderer) {
        // Use rasterization path
        m_RasterRenderer->beginRenderPass(commandBuffer, m_CurrentImageIndex);
        m_RasterRenderer->renderWorld(commandBuffer, world, camera);
        // Don't end render pass yet - UI will render in the same pass
    }
    
    (void)deltaTime;
}

void VulkanBackend::renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) {
    // Placeholder: Entity rendering
    (void)entityRenderer;
    (void)camera;
    (void)alpha;
}

void VulkanBackend::renderUI() {
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(m_Context->getCurrentFrame());
    
    if (m_RayTracingEnabled && m_RTRenderer) {
        // RT mode: Composite RT output to swapchain, then render ImGui
        VkExtent2D extent = m_Context->getSwapchainExtent();
        VkRenderPass renderPass = m_RasterRenderer->getRenderPass();
        VkFramebuffer framebuffer = m_RasterRenderer->getFramebuffer(m_CurrentImageIndex);
        
        // Composite RT output (begins render pass)
        m_RTRenderer->composite(commandBuffer, renderPass, framebuffer, extent.width, extent.height);
        
        // Render ImGui in the same pass
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData) {
            ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
        }
        
        m_RasterRenderer->endRenderPass(commandBuffer);
    } else {
        // Raster mode: ImGui renders in the same pass as world rendering
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData) {
            ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
        }
        
        // End the render pass started in renderWorld
        if (m_RasterRenderer) {
            m_RasterRenderer->endRenderPass(commandBuffer);
        }
    }
}

RenderBackendType VulkanBackend::getBackendType() const {
    return m_RayTracingEnabled ? RenderBackendType::VULKAN_RT : RenderBackendType::VULKAN;
}

void VulkanBackend::handleSwapchainRecreation() {
    PC_INFO("Recreating swapchain");
    m_Context->recreateSwapchain();
}

bool VulkanBackend::initializeImGui() {
    // Create descriptor pool for ImGui
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(m_Context->getDevice(), &poolInfo, nullptr, &m_ImGuiDescriptorPool) != VK_SUCCESS) {
        PC_ERROR("Failed to create ImGui descriptor pool");
        return false;
    }

    // Initialize ImGui Vulkan backend
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_Context->getInstance();
    initInfo.PhysicalDevice = m_Context->getPhysicalDevice();
    initInfo.Device = m_Context->getDevice();
    initInfo.QueueFamily = 0; // Assuming graphics queue family is 0
    initInfo.Queue = m_Context->getGraphicsQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_ImGuiDescriptorPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = m_Context->getSwapchainImageCount();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    if (m_RasterRenderer) {
        ImGui_ImplVulkan_Init(&initInfo, m_RasterRenderer->getRenderPass());
    } else {
        PC_WARN("No render pass available for ImGui initialization");
        return false;
    }

    // Upload fonts
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(0);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(m_Context->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_Context->getGraphicsQueue());
    
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    PC_INFO("ImGui Vulkan backend initialized");
    return true;
}

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

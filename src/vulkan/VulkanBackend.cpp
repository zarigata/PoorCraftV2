#ifdef POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/vulkan/VulkanBackend.h"
#include "poorcraft/vulkan/VulkanContext.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/core/Logger.h"

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

    m_Initialized = true;
    PC_INFO("Vulkan backend initialized successfully");
    return true;
}

void VulkanBackend::shutdown() {
    PC_INFO("Shutting down Vulkan backend");
    
    if (m_Context) {
        m_Context->shutdown();
        m_Context.reset();
    }
    
    m_Initialized = false;
}

void VulkanBackend::beginFrame() {
    VkResult result = m_Context->acquireNextImage(m_CurrentImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleSwapchainRecreation();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        PC_ERROR("Failed to acquire swapchain image");
        return;
    }

    // Begin command buffer recording
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(m_Context->getCurrentFrame());
    
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
    VkCommandBuffer commandBuffer = m_Context->getCommandBuffer(m_Context->getCurrentFrame());
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        PC_ERROR("Failed to record command buffer");
        return;
    }

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_Context->getCommandBuffer(0)}; // Placeholder
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_Context->getCommandBuffer(0)}; // Placeholder
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_Context->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        PC_ERROR("Failed to submit draw command buffer");
    }

    // Present
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
    // Placeholder: Vulkan world rendering not fully implemented yet
    // This would call RTRenderer for ray tracing or VulkanRasterRenderer for rasterization
    (void)world;
    (void)camera;
    (void)deltaTime;
    
    PC_TRACE("Vulkan world rendering (placeholder)");
}

void VulkanBackend::renderEntities(EntityRenderer& entityRenderer, Camera& camera, float alpha) {
    // Placeholder: Entity rendering
    (void)entityRenderer;
    (void)camera;
    (void)alpha;
}

void VulkanBackend::renderUI() {
    // Placeholder: UI rendering via ImGui Vulkan backend
    PC_TRACE("Vulkan UI rendering (placeholder)");
}

RenderBackendType VulkanBackend::getBackendType() const {
    return m_RayTracingEnabled ? RenderBackendType::VULKAN_RT : RenderBackendType::VULKAN;
}

void VulkanBackend::handleSwapchainRecreation() {
    PC_INFO("Recreating swapchain");
    m_Context->recreateSwapchain();
}

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

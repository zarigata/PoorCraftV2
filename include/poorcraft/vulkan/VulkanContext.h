#pragma once

#ifdef POORCRAFT_VULKAN_SUPPORT

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace PoorCraft {

// Forward declarations
class Window;

/**
 * Vulkan context managing core Vulkan objects
 * Handles instance, device, swapchain, command buffers, and synchronization
 */
class VulkanContext {
public:
    explicit VulkanContext(Window& window);
    ~VulkanContext() = default;

    // Non-copyable, non-movable
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // Lifecycle
    bool initialize(bool enableValidation, bool enableRayTracing);
    void shutdown();

    // Swapchain operations
    void recreateSwapchain();
    VkResult acquireNextImage(uint32_t& imageIndex);
    VkResult presentImage(uint32_t imageIndex);

    // Getters
    VkInstance getInstance() const { return m_Instance; }
    VkDevice getDevice() const { return m_Device; }
    VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
    VkSurfaceKHR getSurface() const { return m_Surface; }
    VkSwapchainKHR getSwapchain() const { return m_Swapchain; }
    VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
    VkQueue getPresentQueue() const { return m_PresentQueue; }
    VkCommandPool getCommandPool() const { return m_CommandPool; }
    VkCommandBuffer getCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }
    uint32_t getSwapchainImageCount() const { return static_cast<uint32_t>(m_SwapchainImages.size()); }
    VkFormat getSwapchainFormat() const { return m_SwapchainImageFormat; }
    VkExtent2D getSwapchainExtent() const { return m_SwapchainExtent; }
    VkImageView getSwapchainImageView(uint32_t index) const { return m_SwapchainImageViews[index]; }
    bool supportsRayTracing() const { return m_RayTracingEnabled; }
    uint32_t getCurrentFrame() const { return m_CurrentFrame; }
    VkSemaphore getImageAvailableSemaphore(uint32_t index) const { return m_ImageAvailableSemaphores[index]; }
    VkSemaphore getRenderFinishedSemaphore(uint32_t index) const { return m_RenderFinishedSemaphores[index]; }
    VkFence getInFlightFence(uint32_t index) const { return m_InFlightFences[index]; }

private:
    // Initialization helpers
    bool createInstance(bool enableValidation);
    bool selectPhysicalDevice(bool preferRayTracing);
    bool createLogicalDevice(bool enableRayTracing);
    bool createSwapchain();
    bool createCommandBuffers();
    bool createSyncObjects();

    // Helper functions
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkRayTracingSupport(VkPhysicalDevice device);
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // Queue family indices
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        bool isComplete() const {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // Member variables
    Window& m_Window;
    
    // Vulkan core objects
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    
    // Queues
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    
    // Swapchain
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;
    
    // Command buffers
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    
    // Synchronization
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    uint32_t m_CurrentFrame = 0;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    
    // Debug
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    
    // Features
    bool m_RayTracingEnabled = false;
};

} // namespace PoorCraft

#endif // POORCRAFT_VULKAN_SUPPORT

#include "poorcraft/rendering/RenderBackend.h"
#include "poorcraft/rendering/OpenGLBackend.h"
#include "poorcraft/core/Logger.h"

#ifdef POORCRAFT_VULKAN_SUPPORT
#include "poorcraft/vulkan/VulkanBackend.h"
#endif

namespace PoorCraft {

std::unique_ptr<IRenderBackend> RenderBackendFactory::create(RenderBackendType type, Window& window) {
    switch (type) {
        case RenderBackendType::OPENGL:
            PC_INFO("Creating OpenGL rendering backend");
            return std::make_unique<OpenGLBackend>(window);

        case RenderBackendType::VULKAN:
        case RenderBackendType::VULKAN_RT:
#ifdef POORCRAFT_VULKAN_SUPPORT
            {
                bool enableRT = (type == RenderBackendType::VULKAN_RT);
                PC_INFO("Creating Vulkan rendering backend (RT: {})", enableRT ? "enabled" : "disabled");
                return std::make_unique<VulkanBackend>(window, enableRT);
            }
#else
            PC_ERROR("Vulkan support not compiled. Falling back to OpenGL");
            PC_ERROR("Install Vulkan SDK and rebuild to enable Vulkan backend");
            return std::make_unique<OpenGLBackend>(window);
#endif

        default:
            PC_ERROR("Invalid backend type. Falling back to OpenGL");
            return std::make_unique<OpenGLBackend>(window);
    }
}

} // namespace PoorCraft

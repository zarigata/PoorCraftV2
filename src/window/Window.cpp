#include "poorcraft/window/Window.h"
#include "poorcraft/events/WindowEvent.h"
#include "poorcraft/events/InputEvent.h"
#include "poorcraft/core/Logger.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace PoorCraft {

static void glfwErrorCallback(int error, const char* description) {
    PC_ERROR("[GLFW] Error " + std::to_string(error) + ": " + std::string(description));
}

bool Window::initializeGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        PC_ERROR("[Window] Failed to initialize GLFW");
        return false;
    }

    PC_INFO("[Window] GLFW initialized successfully");
    return true;
}

void Window::terminateGLFW() {
    glfwTerminate();
    PC_INFO("[Window] GLFW terminated");
}

std::vector<Monitor> Window::getMonitors() {
    std::vector<Monitor> monitors;
    
    int count;
    GLFWmonitor** glfwMonitors = glfwGetMonitors(&count);
    
    for (int i = 0; i < count; ++i) {
        Monitor monitor;
        monitor.id = i;
        monitor.name = glfwGetMonitorName(glfwMonitors[i]);
        
        glfwGetMonitorPos(glfwMonitors[i], &monitor.x, &monitor.y);
        
        const GLFWvidmode* mode = glfwGetVideoMode(glfwMonitors[i]);
        if (mode) {
            monitor.width = mode->width;
            monitor.height = mode->height;
            monitor.refreshRate = mode->refreshRate;
            
            // Get all video modes
            int modeCount;
            const GLFWvidmode* modes = glfwGetVideoModes(glfwMonitors[i], &modeCount);
            for (int j = 0; j < modeCount; ++j) {
                Monitor::VideoMode vm;
                vm.width = modes[j].width;
                vm.height = modes[j].height;
                vm.redBits = modes[j].redBits;
                vm.greenBits = modes[j].greenBits;
                vm.blueBits = modes[j].blueBits;
                vm.refreshRate = modes[j].refreshRate;
                monitor.videoModes.push_back(vm);
            }
        }
        
        monitors.push_back(monitor);
    }
    
    return monitors;
}

Monitor Window::getPrimaryMonitor() {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    
    Monitor monitor;
    monitor.id = 0;
    monitor.name = glfwGetMonitorName(primary);
    
    glfwGetMonitorPos(primary, &monitor.x, &monitor.y);
    
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    if (mode) {
        monitor.width = mode->width;
        monitor.height = mode->height;
        monitor.refreshRate = mode->refreshRate;
    }
    
    return monitor;
}

Window::Window(const WindowProperties& props)
    : m_Properties(props)
    , m_WindowedWidth(props.width)
    , m_WindowedHeight(props.height) {
}

Window::~Window() {
    shutdown();
}

Window::Window(Window&& other) noexcept
    : m_Window(other.m_Window)
    , m_Properties(std::move(other.m_Properties))
    , m_EventCallback(std::move(other.m_EventCallback))
    , m_LastMouseX(other.m_LastMouseX)
    , m_LastMouseY(other.m_LastMouseY)
    , m_FirstMouse(other.m_FirstMouse)
    , m_WindowedWidth(other.m_WindowedWidth)
    , m_WindowedHeight(other.m_WindowedHeight) {
    other.m_Window = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        shutdown();
        
        m_Window = other.m_Window;
        m_Properties = std::move(other.m_Properties);
        m_EventCallback = std::move(other.m_EventCallback);
        m_LastMouseX = other.m_LastMouseX;
        m_LastMouseY = other.m_LastMouseY;
        m_FirstMouse = other.m_FirstMouse;
        m_WindowedWidth = other.m_WindowedWidth;
        m_WindowedHeight = other.m_WindowedHeight;
        
        other.m_Window = nullptr;
    }
    return *this;
}

bool Window::initialize() {
    // Set window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Get monitor for fullscreen
    GLFWmonitor* monitor = nullptr;
    if (m_Properties.fullscreen) {
        if (m_Properties.monitorIndex >= 0) {
            int count;
            GLFWmonitor** monitors = glfwGetMonitors(&count);
            if (m_Properties.monitorIndex < count) {
                monitor = monitors[m_Properties.monitorIndex];
            }
        }
        if (!monitor) {
            monitor = glfwGetPrimaryMonitor();
        }
    }
    
    // Create window
    m_Window = glfwCreateWindow(
        m_Properties.width,
        m_Properties.height,
        m_Properties.title.c_str(),
        monitor,
        nullptr
    );
    
    if (!m_Window) {
        PC_ERROR("[Window] Failed to create GLFW window");
        return false;
    }
    
    glfwMakeContextCurrent(m_Window);
    
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        PC_ERROR("[Window] Failed to initialize GLAD");
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        return false;
    }
    
    // Set VSync
    glfwSwapInterval(m_Properties.vsync ? 1 : 0);
    
    // Setup callbacks
    setupCallbacks();
    
    // Update viewport
    updateViewport();
    
    PC_INFO("[Window] Window created: " + std::to_string(m_Properties.width) + "x" +
        std::to_string(m_Properties.height) + " (" +
        (m_Properties.fullscreen ? "Fullscreen" : "Windowed") + ")");

    PC_INFO(std::string("[Window] OpenGL Version: ") + reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    PC_INFO(std::string("[Window] GPU: ") + reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    
    return true;
}

void Window::setupCallbacks() {
    glfwSetWindowUserPointer(m_Window, this);
    
    // Window close callback
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            WindowCloseEvent event;
            win->m_EventCallback(event);
        }
    });
    
    // Window resize callback
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Properties.width = width;
        win->m_Properties.height = height;
        win->updateViewport();
        
        if (win->m_EventCallback) {
            WindowResizeEvent event(width, height);
            win->m_EventCallback(event);
        }
    });
    
    // Window focus callback
    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            WindowFocusEvent event(focused == GLFW_TRUE);
            win->m_EventCallback(event);
        }
    });
    
    // Window iconify callback
    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int iconified) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            WindowMinimizeEvent event(iconified == GLFW_TRUE);
            win->m_EventCallback(event);
        }
    });
    
    // Window position callback
    glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xpos, int ypos) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            WindowMoveEvent event(xpos, ypos);
            win->m_EventCallback(event);
        }
    });
    
    // Key callback
    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            if (action == GLFW_PRESS) {
                KeyPressEvent event(key, false);
                win->m_EventCallback(event);
            } else if (action == GLFW_RELEASE) {
                KeyReleaseEvent event(key);
                win->m_EventCallback(event);
            } else if (action == GLFW_REPEAT) {
                KeyPressEvent event(key, true);
                win->m_EventCallback(event);
            }
        }
    });
    
    // Mouse button callback
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            if (action == GLFW_PRESS) {
                MouseButtonPressEvent event(button);
                win->m_EventCallback(event);
            } else if (action == GLFW_RELEASE) {
                MouseButtonReleaseEvent event(button);
                win->m_EventCallback(event);
            }
        }
    });
    
    // Cursor position callback
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        
        double deltaX = 0.0;
        double deltaY = 0.0;
        
        if (win->m_FirstMouse) {
            win->m_LastMouseX = xpos;
            win->m_LastMouseY = ypos;
            win->m_FirstMouse = false;
        } else {
            deltaX = xpos - win->m_LastMouseX;
            deltaY = ypos - win->m_LastMouseY;
        }
        
        win->m_LastMouseX = xpos;
        win->m_LastMouseY = ypos;
        
        if (win->m_EventCallback) {
            MouseMoveEvent event(xpos, ypos, deltaX, deltaY);
            win->m_EventCallback(event);
        }
    });
    
    // Scroll callback
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win->m_EventCallback) {
            MouseScrollEvent event(xoffset, yoffset);
            win->m_EventCallback(event);
        }
    });
}

void Window::shutdown() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
        PC_INFO("[Window] Window destroyed");
    }
}

bool Window::isOpen() const {
    return m_Window && !glfwWindowShouldClose(m_Window);
{{ ... }}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    if (m_Window) {
        glfwSwapBuffers(m_Window);
    }
}

void Window::setEventCallback(std::function<void(Event&)> callback) {
    m_EventCallback = std::move(callback);
}

void Window::setTitle(const std::string& title) {
    m_Properties.title = title;
    if (m_Window) {
        glfwSetWindowTitle(m_Window, title.c_str());
    }
}

void Window::setSize(uint32_t width, uint32_t height) {
    m_Properties.width = width;
    m_Properties.height = height;
    if (m_Window) {
        glfwSetWindowSize(m_Window, width, height);
        updateViewport();
    }
    if (!m_Properties.fullscreen) {
        m_WindowedWidth = width;
        m_WindowedHeight = height;
    }
}

void Window::setFullscreen(bool fullscreen) {
    if (m_Properties.fullscreen == fullscreen || !m_Window) {
        return;
    }
    
    bool wasFullscreen = m_Properties.fullscreen;
    m_Properties.fullscreen = fullscreen;
    
    if (fullscreen) {
        if (!wasFullscreen) {
            m_WindowedWidth = m_Properties.width;
            m_WindowedHeight = m_Properties.height;
        }

        GLFWmonitor* monitor = nullptr;
        int monitorCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        if (m_Properties.monitorIndex >= 0 && m_Properties.monitorIndex < monitorCount) {
            monitor = monitors[m_Properties.monitorIndex];
        }

        if (!monitor) {
            monitor = glfwGetPrimaryMonitor();
        }

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode) {
            m_Properties.width = mode->width;
            m_Properties.height = mode->height;
            glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            updateViewport();
        }
    } else {
        uint32_t targetWidth = m_WindowedWidth > 0 ? m_WindowedWidth : m_Properties.width;
        uint32_t targetHeight = m_WindowedHeight > 0 ? m_WindowedHeight : m_Properties.height;
        m_Properties.width = targetWidth;
        m_Properties.height = targetHeight;
        glfwSetWindowMonitor(m_Window, nullptr, 100, 100, targetWidth, targetHeight, 0);
        updateViewport();
    }
    
    PC_INFO(std::string("[Window] Fullscreen ") + (fullscreen ? "enabled" : "disabled"));
}

void Window::setVSync(bool vsync) {
    m_Properties.vsync = vsync;
    glfwSwapInterval(vsync ? 1 : 0);
    PC_INFO(std::string("[Window] VSync ") + (vsync ? "enabled" : "disabled"));
}

void Window::setPosition(int x, int y) {
    if (m_Window) {
        glfwSetWindowPos(m_Window, x, y);
    }
}

void Window::updateViewport() {
    if (m_Window) {
        glViewport(0, 0, m_Properties.width, m_Properties.height);
    }
}

} // namespace PoorCraft

#pragma once

#include "poorcraft/core/Event.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

struct GLFWwindow;
struct GLFWmonitor;

namespace PoorCraft {

struct WindowProperties {
    std::string title = "PoorCraft";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool fullscreen = false;
    bool vsync = true;
    int monitorIndex = -1; // -1 for primary monitor
};

struct Monitor {
    int id;
    std::string name;
    int x, y;           // Position
    int width, height;  // Size
    int refreshRate;
    
    struct VideoMode {
        int width, height;
        int redBits, greenBits, blueBits;
        int refreshRate;
    };
    std::vector<VideoMode> videoModes;
};

class Window {
public:
    explicit Window(const WindowProperties& props);
    ~Window();

    // Lifecycle
    bool initialize();
    void shutdown();
    bool isOpen() const;

    // Event handling
    void pollEvents();
    void swapBuffers();
    void setEventCallback(std::function<void(Event&)> callback);

    // Getters
    uint32_t getWidth() const { return m_Properties.width; }
    uint32_t getHeight() const { return m_Properties.height; }
    const std::string& getTitle() const { return m_Properties.title; }
    bool isFullscreen() const { return m_Properties.fullscreen; }
    bool isVSync() const { return m_Properties.vsync; }
    GLFWwindow* getNativeWindow() const { return m_Window; }

    // Setters
    void setTitle(const std::string& title);
    void setSize(uint32_t width, uint32_t height);
    void setFullscreen(bool fullscreen);
    void setVSync(bool vsync);
    void setPosition(int x, int y);

    // Static GLFW management
    static bool initializeGLFW();
    static void terminateGLFW();
    static std::vector<Monitor> getMonitors();
    static Monitor getPrimaryMonitor();

    // Non-copyable but movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

private:
    void setupCallbacks();
    void updateViewport();

    GLFWwindow* m_Window = nullptr;
    WindowProperties m_Properties;
    std::function<void(Event&)> m_EventCallback;
    
    // For mouse delta calculation
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;
    bool m_FirstMouse = true;

    uint32_t m_WindowedWidth = 0;
    uint32_t m_WindowedHeight = 0;
};

} // namespace PoorCraft

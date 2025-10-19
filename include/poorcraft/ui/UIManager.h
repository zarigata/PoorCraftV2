#pragma once

#include <cstdint>

#include <imgui.h>

namespace PoorCraft {

class Window;

class UIManager {
public:
    static UIManager& getInstance();

    void initialize(Window& window);
    void shutdown();

    bool isInitialized() const { return m_Initialized; }

    void beginFrame();
    void endFrame();

    bool wantCaptureMouse() const;
    bool wantCaptureKeyboard() const;

    void setMouseCursor(bool visible);

private:
    UIManager() = default;
    ~UIManager() = default;

    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    bool m_Initialized = false;
    Window* m_Window = nullptr;
};

} // namespace PoorCraft

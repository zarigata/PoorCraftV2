#pragma once

#include <functional>
#include <chrono>

namespace PoorCraft {

class Window;

class GameLoop {
public:
    using UpdateCallback = std::function<void(float)>;
    using RenderCallback = std::function<void()>;

    explicit GameLoop(Window& window);
    ~GameLoop() = default;

    // Main loop control
    void run();
    void stop();

    // Callbacks
    void setUpdateCallback(UpdateCallback callback);
    void setRenderCallback(RenderCallback callback);

    // Timing configuration
    void setFixedTimestep(float timestep);
    void setMaxFPS(int maxFPS);

    // Getters
    float getFPS() const { return m_FPS; }
    float getFrameTime() const { return m_FrameTime; }
    float getUpdateTime() const { return m_UpdateTime; }
    float getRenderTime() const { return m_RenderTime; }
    bool isRunning() const { return m_Running; }

    // Non-copyable, non-movable
    GameLoop(const GameLoop&) = delete;
    GameLoop& operator=(const GameLoop&) = delete;
    GameLoop(GameLoop&&) = delete;
    GameLoop& operator=(GameLoop&&) = delete;

private:
    Window& m_Window;
    UpdateCallback m_UpdateCallback;
    RenderCallback m_RenderCallback;

    bool m_Running = false;
    float m_FixedTimestep = 1.0f / 60.0f; // 60 FPS default
    int m_MaxFPS = 0; // 0 = unlimited

    // Timing state
    std::chrono::high_resolution_clock::time_point m_LastFrameTime = {};
    double m_Accumulator = 0.0;
    
    // Performance metrics
    float m_FPS = 0.0f;
    float m_FrameTime = 0.0f;
    float m_UpdateTime = 0.0f;
    float m_RenderTime = 0.0f;
    
    // FPS calculation
    int m_FrameCount = 0;
    double m_FPSTimer = 0.0;
};

} // namespace PoorCraft

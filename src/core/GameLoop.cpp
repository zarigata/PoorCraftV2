#include "poorcraft/core/GameLoop.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/input/Input.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/core/Logger.h"

#include <chrono>
#include <thread>

namespace PoorCraft {

GameLoop::GameLoop(Window& window)
    : m_Window(window) {
}

void GameLoop::setUpdateCallback(UpdateCallback callback) {
    m_UpdateCallback = std::move(callback);
}

void GameLoop::setRenderCallback(RenderCallback callback) {
    m_RenderCallback = std::move(callback);
}

void GameLoop::setFixedTimestep(float timestep) {
    m_FixedTimestep = timestep;
    PC_INFO("[GameLoop] Fixed timestep set to: " + std::to_string(timestep) + "s (" +
        std::to_string(1.0f / timestep) + " updates/sec)");
}

void GameLoop::setMaxFPS(int maxFPS) {
    m_MaxFPS = maxFPS;
    if (maxFPS > 0) {
        PC_INFO("[GameLoop] Max FPS set to: " + std::to_string(maxFPS));
    } else {
        PC_INFO("[GameLoop] Max FPS: Unlimited");
    }
}

void GameLoop::run() {
    m_Running = true;
    m_LastFrameTime = poorcraft::Platform::get_time();
    m_Accumulator = 0.0;
    m_FrameCount = 0;
    m_FPSTimer = 0.0;

    PC_INFO("[GameLoop] Game loop started");

    while (m_Window.isOpen() && m_Running) {
        // Calculate frame time
        auto now = poorcraft::Platform::get_time();
        double frameTime = std::chrono::duration<double>(now - m_LastFrameTime).count();
        m_LastFrameTime = now;
        m_FrameTime = static_cast<float>(frameTime);

        // Add to accumulator for fixed timestep updates
        m_Accumulator += frameTime;

        // Poll window events
        m_Window.pollEvents();
        
        // Process queued events
        EventBus::getInstance().processEvents();
        
        // Fixed timestep updates
        auto updateStartTime = poorcraft::Platform::get_time();
        int updateCount = 0;
        const int maxUpdates = 5; // Prevent spiral of death

        while (m_Accumulator >= m_FixedTimestep && updateCount < maxUpdates) {
            if (m_UpdateCallback) {
                m_UpdateCallback(m_FixedTimestep);
            }
            m_Accumulator -= m_FixedTimestep;
            updateCount++;
        }

        // If we hit max updates, reset accumulator to prevent spiral
        if (updateCount >= maxUpdates) {
            m_Accumulator = 0.0;
            PC_WARN("[GameLoop] Update spiral detected, resetting accumulator");
        }

        auto updateEndTime = poorcraft::Platform::get_time();
        m_UpdateTime = static_cast<float>(std::chrono::duration<double>(updateEndTime - updateStartTime).count());

        // Render
        auto renderStartTime = poorcraft::Platform::get_time();
        if (m_RenderCallback) {
            m_RenderCallback();
        }
        auto renderEndTime = poorcraft::Platform::get_time();
        m_RenderTime = static_cast<float>(std::chrono::duration<double>(renderEndTime - renderStartTime).count());

        // Swap buffers
        m_Window.swapBuffers();

        // Update input system at end of frame
        Input::getInstance().update();

        // Calculate FPS
        m_FrameCount++;
        m_FPSTimer += frameTime;

        if (m_FPSTimer >= 1.0) {
            m_FPS = static_cast<float>(m_FrameCount) / static_cast<float>(m_FPSTimer);
            PC_DEBUG("[GameLoop] FPS: " + std::to_string(static_cast<int>(m_FPS)) +
                " | Frame: " + std::to_string(m_FrameTime * 1000.0f) + "ms" +
                " | Update: " + std::to_string(m_UpdateTime * 1000.0f) + "ms" +
                " | Render: " + std::to_string(m_RenderTime * 1000.0f) + "ms");
            m_FrameCount = 0;
            m_FPSTimer = 0.0;
        }

        // Frame rate limiting
        if (m_MaxFPS > 0) {
            double targetFrameTime = 1.0 / m_MaxFPS;
            auto endTimePoint = poorcraft::Platform::get_time();
            double actualFrameTime = std::chrono::duration<double>(endTimePoint - now).count();
            double sleepTime = targetFrameTime - actualFrameTime;

            if (sleepTime > 0.0) {
                poorcraft::Platform::sleep(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::duration<double>(sleepTime)));
            }
        }
    }

    PC_INFO("[GameLoop] Game loop stopped");
    m_Running = false;
}

void GameLoop::stop() {
    m_Running = false;
    PC_INFO("[GameLoop] Stop requested");
}

} // namespace PoorCraft

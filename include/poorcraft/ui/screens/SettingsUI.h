#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <string>
#include <vector>

namespace PoorCraft {

using Config = ::poorcraft::Config;
class Window;
class Renderer;
class GameStateManager;

class SettingsUI : public UIScreen {
public:
    SettingsUI(Config& config, Window& window, Renderer& renderer, GameStateManager& stateManager);

    void onEnter() override;
    void render() override;

private:
    void applyGraphicsSettings();
    void applyAudioSettings();
    void applyControlSettings();

    void loadCurrentSettings();

    Config& m_Config;
    Window& m_Window;
    Renderer& m_Renderer;
    GameStateManager& m_StateManager;

    int m_TempWidth = 1280;
    int m_TempHeight = 720;
    bool m_TempFullscreen = false;
    bool m_TempVsync = true;
    int m_TempFov = 90;
    int m_TempRenderDistance = 8;

    float m_TempMasterVolume = 1.0f;
    float m_TempMusicVolume = 0.7f;
    float m_TempSoundVolume = 0.8f;

    float m_TempMouseSensitivity = 1.0f;
    bool m_TempInvertY = false;

    std::vector<std::pair<int, int>> m_Resolutions;
};

} // namespace PoorCraft

#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <string>
#include <vector>

namespace PoorCraft {

class GameStateManager;
class World;
using Config = ::poorcraft::Config;

class LoadingScreenUI : public UIScreen {
public:
    LoadingScreenUI(GameStateManager& stateManager, Config& config);

    void onEnter() override;
    void update(float deltaTime) override;
    void render() override;

    void setProgress(float progress, const std::string& tip);
    void bindWorld(World* world);

private:
    GameStateManager& m_StateManager;
    Config& m_Config;
    World* m_World = nullptr;
    float m_Progress = 0.0f;
    std::string m_CurrentTip;
    float m_TipTimer = 0.0f;
    float m_TipInterval = 5.0f;
    std::vector<std::string> m_Tips;
};

} // namespace PoorCraft

#pragma once

#include "poorcraft/ui/UIScreen.h"

namespace PoorCraft {

class GameStateManager;
class NetworkManager;
class Window;

class MainMenuUI : public UIScreen {
public:
    MainMenuUI(GameStateManager& stateManager, NetworkManager& networkManager, Window& window);

    void update(float deltaTime) override;
    void render() override;

private:
    GameStateManager& m_StateManager;
    NetworkManager& m_NetworkManager;
    Window& m_Window;
};

} // namespace PoorCraft

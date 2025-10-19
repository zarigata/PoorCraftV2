#pragma once

#include "poorcraft/ui/UIScreen.h"

namespace PoorCraft {

class GameStateManager;
class NetworkManager;

class PauseMenuUI : public UIScreen {
public:
    PauseMenuUI(GameStateManager& stateManager, NetworkManager& networkManager);

    void render() override;

private:
    GameStateManager& m_StateManager;
    NetworkManager& m_NetworkManager;
};

} // namespace PoorCraft

#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <string>

namespace PoorCraft {

class GameStateManager;
class NetworkManager;
using Config = ::poorcraft::Config;

class MultiplayerMenuUI : public UIScreen {
public:
    MultiplayerMenuUI(GameStateManager& stateManager, NetworkManager& networkManager, Config& config);

    void onEnter() override;
    void render() override;

private:
    void hostIntegratedServer();
    void joinDirect();

    GameStateManager& m_StateManager;
    NetworkManager& m_NetworkManager;
    Config& m_Config;

    char m_PlayerName[64];
    char m_AddressBuffer[256];
    char m_PortBuffer[16];
    bool m_Hosting = false;
    std::string m_StatusMessage;
};

} // namespace PoorCraft

#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <cstdint>
#include <string>
#include <vector>

namespace PoorCraft {

class GameStateManager;
class NetworkManager;
using Config = ::poorcraft::Config;

class ServerBrowserUI : public UIScreen {
public:
    struct ServerEntry {
        std::string name;
        std::string address;
        std::uint16_t port;
        int playerCount;
        int maxPlayers;
        std::uint32_t ping;
        std::string version;
    };

    ServerBrowserUI(GameStateManager& stateManager, NetworkManager& networkManager, Config& config);

    void onEnter() override;
    void update(float deltaTime) override;
    void render() override;

private:
    void refreshServerList();
    void connectToServer(const std::string& address, std::uint16_t port);

    GameStateManager& m_StateManager;
    NetworkManager& m_NetworkManager;
    Config& m_Config;

    std::vector<ServerEntry> m_Servers;
    int m_SelectedServer = -1;

    char m_AddressBuffer[256];
    char m_PortBuffer[16];
};

} // namespace PoorCraft

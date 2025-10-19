#include "poorcraft/ui/screens/MultiplayerMenuUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/ui/GameState.h"

#include <imgui.h>
#include <cstring>

namespace PoorCraft {

namespace {
constexpr std::uint16_t DEFAULT_PORT = 25565;
constexpr std::size_t DEFAULT_MAX_PLAYERS = 10;
}

MultiplayerMenuUI::MultiplayerMenuUI(GameStateManager& stateManager, NetworkManager& networkManager, Config& config)
    : m_StateManager(stateManager), m_NetworkManager(networkManager), m_Config(config) {
    
    // Initialize buffers
    std::memset(m_PlayerName, 0, sizeof(m_PlayerName));
    std::memset(m_AddressBuffer, 0, sizeof(m_AddressBuffer));
    std::memset(m_PortBuffer, 0, sizeof(m_PortBuffer));
    
    // Read player name from config
    std::string playerName = m_Config.get_string(Config::PlayerConfig::NAME_KEY, "Player");
    std::snprintf(m_PlayerName, sizeof(m_PlayerName), "%s", playerName.c_str());
    
    // Set default address and port
    std::snprintf(m_AddressBuffer, sizeof(m_AddressBuffer), "localhost");
    std::snprintf(m_PortBuffer, sizeof(m_PortBuffer), "%u", DEFAULT_PORT);
}

void MultiplayerMenuUI::onEnter() {
    UIScreen::onEnter();
    m_Hosting = false;
    m_StatusMessage.clear();
}

void MultiplayerMenuUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500.0f, 450.0f));
    ImGui::SetNextWindowBgAlpha(0.95f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | 
                                             ImGuiWindowFlags_NoResize | 
                                             ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Multiplayer", nullptr, windowFlags)) {
        ImGui::TextUnformatted("Multiplayer Menu");
        ImGui::Separator();
        ImGui::Spacing();

        // Player name input
        ImGui::TextUnformatted("Player Name:");
        if (ImGui::InputText("##PlayerName", m_PlayerName, sizeof(m_PlayerName))) {
            // Persist player name to config
            m_Config.set_string(Config::PlayerConfig::NAME_KEY, std::string(m_PlayerName));
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Join Server Browser
        if (ImGui::Button("Join Server", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MultiplayerMenuUI: Opening server browser");
            m_StateManager.pushState(GameState::SERVER_BROWSER);
        }

        ImGui::Spacing();

        // Host Server section
        ImGui::TextUnformatted("Host Server:");
        
        static char portBuffer[16];
        static char maxPlayersBuffer[16];
        static bool initialized = false;
        
        if (!initialized) {
            std::snprintf(portBuffer, sizeof(portBuffer), "%u", DEFAULT_PORT);
            std::snprintf(maxPlayersBuffer, sizeof(maxPlayersBuffer), "%zu", DEFAULT_MAX_PLAYERS);
            initialized = true;
        }
        
        ImGui::InputText("Port##Host", portBuffer, sizeof(portBuffer));
        ImGui::InputText("Max Players", maxPlayersBuffer, sizeof(maxPlayersBuffer));

        if (ImGui::Button("Host Server", ImVec2(-FLT_MIN, 0.0f))) {
            std::uint16_t port = DEFAULT_PORT;
            std::size_t maxPlayers = DEFAULT_MAX_PLAYERS;
            
            try {
                port = static_cast<std::uint16_t>(std::stoi(portBuffer));
            } catch (...) {
                PC_WARN("MultiplayerMenuUI: Invalid port, using default");
            }
            
            try {
                maxPlayers = static_cast<std::size_t>(std::stoi(maxPlayersBuffer));
            } catch (...) {
                PC_WARN("MultiplayerMenuUI: Invalid max players, using default");
            }
            
            hostIntegratedServer();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Direct Connect section
        ImGui::TextUnformatted("Direct Connect:");
        ImGui::InputText("Address##Direct", m_AddressBuffer, sizeof(m_AddressBuffer));
        ImGui::InputText("Port##Direct", m_PortBuffer, sizeof(m_PortBuffer));

        if (ImGui::Button("Connect", ImVec2(-FLT_MIN, 0.0f))) {
            joinDirect();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Status message
        if (!m_StatusMessage.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            ImGui::TextWrapped("%s", m_StatusMessage.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        // Back button
        if (ImGui::Button("Back to Main Menu", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MultiplayerMenuUI: Back to main menu");
            m_StateManager.setState(GameState::MAIN_MENU);
        }
    }
    ImGui::End();
}

void MultiplayerMenuUI::hostIntegratedServer() {
    std::uint16_t port = DEFAULT_PORT;
    std::size_t maxPlayers = DEFAULT_MAX_PLAYERS;
    
    try {
        port = static_cast<std::uint16_t>(std::stoi(m_PortBuffer));
    } catch (...) {
        port = DEFAULT_PORT;
    }
    
    PC_INFO("MultiplayerMenuUI: Starting integrated server on port " + std::to_string(port));
    
    if (m_NetworkManager.startIntegratedServer(port, maxPlayers)) {
        m_StatusMessage = "Server started successfully!";
        m_Hosting = true;
        PC_INFO("MultiplayerMenuUI: Integrated server started, transitioning to IN_GAME");
        m_StateManager.setState(GameState::IN_GAME);
    } else {
        m_StatusMessage = "Failed to start server. Check logs.";
        PC_ERROR("MultiplayerMenuUI: Failed to start integrated server");
    }
}

void MultiplayerMenuUI::joinDirect() {
    std::string address = m_AddressBuffer;
    std::string portString = m_PortBuffer;
    std::string playerName = m_PlayerName;
    
    if (address.empty()) {
        m_StatusMessage = "Please enter a server address.";
        PC_WARN("MultiplayerMenuUI: Address is empty");
        return;
    }
    
    if (playerName.empty()) {
        playerName = "Player";
    }
    
    std::uint16_t port = DEFAULT_PORT;
    try {
        port = static_cast<std::uint16_t>(std::stoi(portString));
    } catch (...) {
        PC_WARN("MultiplayerMenuUI: Invalid port, using default");
    }
    
    PC_INFO("MultiplayerMenuUI: Connecting to " + address + ":" + std::to_string(port) + " as " + playerName);
    
    if (m_NetworkManager.connectToServer(address, port, playerName)) {
        m_StatusMessage = "Connecting to server...";
        m_StateManager.setState(GameState::CONNECTING);
    } else {
        m_StatusMessage = "Failed to connect. Check address and port.";
        PC_ERROR("MultiplayerMenuUI: Connection failed");
    }
}

} // namespace PoorCraft

#include "poorcraft/ui/screens/ServerBrowserUI.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/ui/GameState.h"

#include <algorithm>
#include <cstring>

#include <imgui.h>

namespace PoorCraft {

namespace {
constexpr std::uint16_t DEFAULT_PORT = 25565;
constexpr std::size_t MAX_MESSAGES = 100;
} // namespace

ServerBrowserUI::ServerBrowserUI(GameStateManager& stateManager, NetworkManager& networkManager, Config& config)
    : m_StateManager(stateManager), m_NetworkManager(networkManager), m_Config(config) {
    std::memset(m_AddressBuffer, 0, sizeof(m_AddressBuffer));
    std::memset(m_PortBuffer, 0, sizeof(m_PortBuffer));
    std::snprintf(m_AddressBuffer, sizeof(m_AddressBuffer), "%s", "localhost");
    std::snprintf(m_PortBuffer, sizeof(m_PortBuffer), "%u", DEFAULT_PORT);
}

void ServerBrowserUI::onEnter() {
    UIScreen::onEnter();
    refreshServerList();
}

void ServerBrowserUI::update(float) {}

void ServerBrowserUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f));

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Server Browser", nullptr, windowFlags)) {
        if (ImGui::Button("Refresh", ImVec2(120.0f, 0.0f))) {
            refreshServerList();
        }
        ImGui::SameLine();
        if (ImGui::Button("Back", ImVec2(120.0f, 0.0f))) {
            m_StateManager.popState();
            ImGui::End();
            return;
        }

        ImGui::Separator();

        constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("ServerTable", 5, tableFlags)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Players", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Ping", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Version", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (int i = 0; i < static_cast<int>(m_Servers.size()); ++i) {
                const auto& entry = m_Servers[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                bool selected = (i == m_SelectedServer);
                if (ImGui::Selectable(entry.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    m_SelectedServer = i;
                    PC_INFO("ServerBrowserUI: Selected server " + entry.name);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted((entry.address + ":" + std::to_string(entry.port)).c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d / %d", entry.playerCount, entry.maxPlayers);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%u ms", entry.ping);

                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(entry.version.c_str());
            }

            ImGui::EndTable();
        }

        ImGui::Separator();

        ImGui::TextUnformatted("Direct Connect");
        ImGui::InputText("Address", m_AddressBuffer, sizeof(m_AddressBuffer));
        ImGui::InputText("Port", m_PortBuffer, sizeof(m_PortBuffer));

        if (ImGui::Button("Connect", ImVec2(120.0f, 0.0f))) {
            std::string address = m_AddressBuffer;
            std::string portString = m_PortBuffer;

            if (address.empty()) {
                PC_WARN("ServerBrowserUI: Address is empty");
            } else {
                std::uint16_t port = DEFAULT_PORT;
                try {
                    port = static_cast<std::uint16_t>(std::stoi(portString));
                } catch (const std::exception&) {
                    PC_WARN("ServerBrowserUI: Invalid port, using default");
                }
                connectToServer(address, port);
            }
        }

        if (m_SelectedServer >= 0 && m_SelectedServer < static_cast<int>(m_Servers.size())) {
            ImGui::SameLine();
            if (ImGui::Button("Connect to Selected", ImVec2(200.0f, 0.0f))) {
                const auto& entry = m_Servers[m_SelectedServer];
                connectToServer(entry.address, entry.port);
            }
        }
    }
    ImGui::End();
}

void ServerBrowserUI::refreshServerList() {
    m_Servers.clear();

    ServerEntry local;
    local.name = "Local Server";
    local.address = "localhost";
    local.port = DEFAULT_PORT;
    local.playerCount = 0;
    local.maxPlayers = m_Config.get_int(Config::NetworkConfig::MAX_PLAYERS_KEY, 10);
    local.ping = 0;
    local.version = "0.1.0";

    m_Servers.push_back(local);
    m_SelectedServer = !m_Servers.empty() ? 0 : -1;

    PC_INFO("ServerBrowserUI: Server list refreshed");
}

void ServerBrowserUI::connectToServer(const std::string& address, std::uint16_t port) {
    std::string playerName = m_Config.get_string("Player.name", "Player");
    PC_INFO("ServerBrowserUI: Connecting to " + address + ":" + std::to_string(port));

    if (m_NetworkManager.connectToServer(address, port, playerName)) {
        m_StateManager.setState(GameState::CONNECTING);
    } else {
        PC_ERROR("ServerBrowserUI: Connection failed");
    }
}

} // namespace PoorCraft

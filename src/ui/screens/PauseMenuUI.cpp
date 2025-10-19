#include "poorcraft/ui/screens/PauseMenuUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/ui/GameState.h"
#include "poorcraft/ui/UIScreenManager.h"

#include <imgui.h>

namespace PoorCraft {

PauseMenuUI::PauseMenuUI(GameStateManager& stateManager, NetworkManager& networkManager)
    : m_StateManager(stateManager), m_NetworkManager(networkManager) {}

void PauseMenuUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* drawList = ImGui::GetForegroundDrawList(viewport);
    drawList->AddRectFilled(viewport->Pos, viewport->Pos + viewport->Size, ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.5f)));

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.95f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Game Paused", nullptr, windowFlags)) {
        ImGui::TextUnformatted("Game Paused");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Resume", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("PauseMenuUI: Resume");
            m_StateManager.popState();
            return;
        }

        if (ImGui::Button("Settings", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("PauseMenuUI: Settings");
            m_StateManager.pushState(GameState::SETTINGS);
        }

        if (m_NetworkManager.isClient()) {
            if (ImGui::Button("Disconnect", ImVec2(-FLT_MIN, 0.0f))) {
                PC_INFO("PauseMenuUI: Disconnect");
                m_NetworkManager.disconnect();
                m_StateManager.setState(GameState::MAIN_MENU);
            }
        }

        if (ImGui::Button("Quit to Title", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("PauseMenuUI: Quit to Title");
            m_StateManager.setState(GameState::MAIN_MENU);
        }

        if (ImGui::Button("Quit Game", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("PauseMenuUI: Quit Game");
            PoorCraft::UIScreenManager::getInstance().requestCloseApplication();
        }
    }
    ImGui::End();
}

} // namespace PoorCraft

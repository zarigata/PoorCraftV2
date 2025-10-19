#include "poorcraft/ui/screens/MainMenuUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/ui/GameState.h"
#include "poorcraft/window/Window.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

namespace PoorCraft {

MainMenuUI::MainMenuUI(GameStateManager& stateManager, NetworkManager& networkManager, Window& window)
    : m_StateManager(stateManager), m_NetworkManager(networkManager), m_Window(window) {}

void MainMenuUI::update(float) {}

void MainMenuUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f));

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("PoorCraft Main Menu", nullptr, windowFlags)) {
        ImGui::Spacing();
        ImGui::TextUnformatted("PoorCraft v0.1.0");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Singleplayer", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MainMenuUI: Singleplayer selected");
            m_StateManager.setState(GameState::SINGLEPLAYER_LOADING);
        }

        if (ImGui::Button("Multiplayer", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MainMenuUI: Multiplayer selected");
            m_StateManager.setState(GameState::MULTIPLAYER_MENU);
        }

        if (ImGui::Button("Settings", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MainMenuUI: Settings selected");
            m_StateManager.pushState(GameState::SETTINGS);
        }

        if (ImGui::Button("Quit", ImVec2(-FLT_MIN, 0.0f))) {
            PC_INFO("MainMenuUI: Quit selected");
            GLFWwindow* nativeWindow = m_Window.getNativeWindow();
            if (nativeWindow) {
                glfwSetWindowShouldClose(nativeWindow, GLFW_TRUE);
            }
        }
    }
    ImGui::End();
}

} // namespace PoorCraft

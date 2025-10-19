#include "poorcraft/ui/screens/LoadingScreenUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/ui/GameState.h"
#include "poorcraft/world/World.h"

#include <imgui.h>
#include <cmath>

namespace PoorCraft {

LoadingScreenUI::LoadingScreenUI(GameStateManager& stateManager, Config& config)
    : m_StateManager(stateManager), m_Config(config) {
    
    // Seed loading tips
    m_Tips = {
        "Tip: Press F3 to toggle debug information",
        "Tip: Use WASD to move and Space to jump",
        "Tip: Hold Shift to sprint",
        "Tip: Press E to open your inventory",
        "Tip: Press T to open chat in multiplayer",
        "Tip: Press Escape to pause the game",
        "Tip: Mine blocks by left-clicking",
        "Tip: Place blocks by right-clicking",
        "Tip: Diamonds are found below Y level 16",
        "Tip: Always carry a crafting table",
        "Tip: Build a shelter before nightfall",
        "Tip: Torches prevent monster spawns",
        "Tip: Use F5 to change camera perspective",
        "Tip: Sneak with Left Ctrl to prevent falling",
        "Tip: Water can break your fall from any height"
    };
    
    m_Progress = 0.0f;
    m_TipTimer = 0.0f;
    m_TipInterval = 5.0f;
    
    // Select initial random tip
    if (!m_Tips.empty()) {
        m_CurrentTip = m_Tips[0];
    }
}

void LoadingScreenUI::onEnter() {
    UIScreen::onEnter();
    m_Progress = 0.0f;
    m_TipTimer = 0.0f;
    
    // Select random tip
    if (!m_Tips.empty()) {
        int randomIndex = rand() % m_Tips.size();
        m_CurrentTip = m_Tips[randomIndex];
    }
}

void LoadingScreenUI::update(float deltaTime) {
    m_TipTimer += deltaTime;
    
    // Rotate tips periodically
    if (m_TipTimer >= m_TipInterval && !m_Tips.empty()) {
        m_TipTimer = 0.0f;
        int randomIndex = rand() % m_Tips.size();
        m_CurrentTip = m_Tips[randomIndex];
    }
    
    // Check if world is ready
    if (m_World) {
        // Check if world initialization is complete
        // For now, we'll use a simple progress threshold
        if (m_Progress >= 0.99f) {
            PC_INFO("LoadingScreenUI: World ready, transitioning to IN_GAME");
            m_StateManager.setState(GameState::IN_GAME);
            return;
        }
    }
    
    // Auto-increment progress if no explicit updates (fallback)
    if (m_Progress < 1.0f) {
        m_Progress += deltaTime * 0.2f; // Slow auto-progress
        if (m_Progress > 1.0f) {
            m_Progress = 1.0f;
        }
    }
}

void LoadingScreenUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Full-screen dark overlay
    ImDrawList* drawList = ImGui::GetForegroundDrawList(viewport);
    drawList->AddRectFilled(viewport->Pos, 
                           ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
                           ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.95f)));

    // Centered loading window
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500.0f, 200.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | 
                                             ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("LoadingScreen", nullptr, windowFlags)) {
        // Title
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Loading World...").x) * 0.5f);
        ImGui::TextUnformatted("Loading World...");
        
        ImGui::Spacing();
        ImGui::Spacing();

        // Progress bar
        ImGui::ProgressBar(m_Progress, ImVec2(-FLT_MIN, 30.0f));
        
        ImGui::Spacing();
        ImGui::Spacing();

        // Loading tip with rotation animation
        if (!m_CurrentTip.empty()) {
            float tipWidth = ImGui::CalcTextSize(m_CurrentTip.c_str()).x;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - tipWidth) * 0.5f);
            
            // Subtle fade animation
            float alpha = 0.7f + 0.3f * std::sin(m_TipTimer * 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, alpha));
            ImGui::TextUnformatted(m_CurrentTip.c_str());
            ImGui::PopStyleColor();
        }
        
        // Spinning indicator
        ImGui::Spacing();
        ImGui::Spacing();
        
        const char* spinnerChars[] = {"|", "/", "-", "\\"};
        int spinnerIndex = static_cast<int>(m_TipTimer * 4.0f) % 4;
        std::string spinner = std::string("Loading ") + spinnerChars[spinnerIndex];
        
        float spinnerWidth = ImGui::CalcTextSize(spinner.c_str()).x;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - spinnerWidth) * 0.5f);
        ImGui::TextUnformatted(spinner.c_str());
    }
    ImGui::End();
}

void LoadingScreenUI::setProgress(float progress, const std::string& tip) {
    m_Progress = progress;
    if (m_Progress > 1.0f) {
        m_Progress = 1.0f;
    }
    if (m_Progress < 0.0f) {
        m_Progress = 0.0f;
    }
    
    if (!tip.empty()) {
        m_CurrentTip = tip;
    }
    
    PC_INFO("LoadingScreenUI: Progress " + std::to_string(static_cast<int>(m_Progress * 100.0f)) + "%");
}

void LoadingScreenUI::bindWorld(World* world) {
    m_World = world;
    PC_INFO("LoadingScreenUI: World bound");
}

} // namespace PoorCraft

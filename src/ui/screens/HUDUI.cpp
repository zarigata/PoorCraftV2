#include "poorcraft/ui/screens/HUDUI.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/GameLoop.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/physics/Player.h"
#include "poorcraft/world/World.h"
#include "poorcraft/world/ChunkManager.h"

#include <imgui.h>

namespace PoorCraft {

namespace {
ImVec4 applyOpacity(const ImVec4& color, float opacity) {
    return ImVec4(color.x, color.y, color.z, color.w * opacity);
}
} // namespace

HUDUI::HUDUI(Config& config)
    : m_Config(config) {}

void HUDUI::bindContext(Player* player, World* world, GameLoop* gameLoop) {
    m_Player = player;
    m_World = world;
    m_GameLoop = gameLoop;
}

void HUDUI::render() {
    if (!isActive()) {
        return;
    }

    if (!m_Player || !m_World || !m_GameLoop) {
        return;
    }

    m_HudOpacity = m_Config.get_float(poorcraft::Config::UIConfig::HUD_OPACITY_KEY, 0.8f);

    drawCrosshair();
    drawHealthBar();
    drawHungerBar();
    drawHotbar();

    bool showDebug = m_Config.get_bool(poorcraft::Config::UIConfig::SHOW_DEBUG_INFO_KEY, false) || m_ShowDebug;
    bool showCoordinates = m_Config.get_bool(poorcraft::Config::UIConfig::SHOW_COORDINATES_KEY, true);

    if (showDebug) {
        drawDebugInfo();
    }

    if (showCoordinates && !showDebug) {
        drawCoordinates();
    }
}

void HUDUI::setShowDebug(bool value) {
    m_ShowDebug = value;
}

bool HUDUI::isDebugVisible() const {
    return m_ShowDebug;
}

void HUDUI::drawCrosshair() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* drawList = ImGui::GetForegroundDrawList(viewport);

    const ImVec2 center = viewport->GetCenter();
    const float size = 10.0f;
    const ImU32 color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.8f));

    drawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y), color, 2.0f);
    drawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), color, 2.0f);
}

void HUDUI::drawHealthBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 20.0f, viewport->Pos.y + viewport->Size.y - 100.0f));
    ImGui::SetNextWindowBgAlpha(m_HudOpacity);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_NoInputs |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUDHealth", nullptr, windowFlags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, applyOpacity(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), m_HudOpacity));
        ImGui::TextUnformatted("Health: ❤ ❤ ❤ ❤ ❤ ❤ ❤ ❤ ❤ ❤");
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void HUDUI::drawHungerBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 20.0f, viewport->Pos.y + viewport->Size.y - 70.0f));
    ImGui::SetNextWindowBgAlpha(m_HudOpacity);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_NoInputs |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUDHunger", nullptr, windowFlags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, applyOpacity(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), m_HudOpacity));
        ImGui::TextUnformatted("Hunger: 🍖 🍖 🍖 🍖 🍖 🍖 🍖 🍖 🍖 🍖");
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void HUDUI::drawHotbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->GetCenter().x - 200.0f, viewport->Pos.y + viewport->Size.y - 80.0f));
    ImGui::SetNextWindowSize(ImVec2(400.0f, 60.0f));
    ImGui::SetNextWindowBgAlpha(m_HudOpacity);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_NoInputs |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUDHotbar", nullptr, windowFlags)) {
        ImGui::Columns(9, nullptr, false);
        for (int i = 0; i < 9; ++i) {
            ImGui::PushID(i);
            bool isSelected = (i == 0);
            ImVec2 cursor = ImGui::GetCursorPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 min = ImVec2(pos.x + cursor.x, pos.y + cursor.y);
            ImVec2 max = ImVec2(min.x + 40.0f, min.y + 40.0f);

            ImU32 borderColor = ImGui::GetColorU32(isSelected ? applyOpacity(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), m_HudOpacity)
                                                              : applyOpacity(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), m_HudOpacity));
            drawList->AddRect(min, max, borderColor, 4.0f, 0, 2.0f);

            ImGui::Dummy(ImVec2(40.0f, 40.0f));
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Columns(1);
    }
    ImGui::End();
}

void HUDUI::drawDebugInfo() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 20.0f, viewport->Pos.y + 20.0f));
    ImGui::SetNextWindowBgAlpha(0.6f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_NoInputs |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUDDebug", nullptr, windowFlags)) {
        float fps = m_GameLoop->getFPS();
        const glm::vec3& pos = m_Player->getPosition();
        const auto& stats = m_World->getRenderStats();

        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Position: %.2f / %.2f / %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("Chunks: %zu rendered / %zu total", stats.chunksRendered, stats.totalChunks);
        ImGui::Text("Vertices: %zu", stats.verticesRendered);
    }
    ImGui::End();
}

void HUDUI::drawCoordinates() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - 230.0f, viewport->Pos.y + 20.0f));
    ImGui::SetNextWindowBgAlpha(0.6f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                             ImGuiWindowFlags_NoInputs |
                                             ImGuiWindowFlags_NoFocusOnAppearing |
                                             ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("HUDCoordinates", nullptr, windowFlags)) {
        const glm::vec3& pos = m_Player->getPosition();
        ImGui::Text("X: %.2f", pos.x);
        ImGui::Text("Y: %.2f", pos.y);
        ImGui::Text("Z: %.2f", pos.z);
    }
    ImGui::End();
}

} // namespace PoorCraft

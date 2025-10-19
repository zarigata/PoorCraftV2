#include "poorcraft/ui/screens/InventoryUI.h"

#include "poorcraft/core/Logger.h"
#include "poorcraft/ui/GameState.h"

#include <imgui.h>

namespace PoorCraft {

InventoryUI::InventoryUI(Config& config, GameStateManager& stateManager)
    : m_Config(config), m_StateManager(stateManager) {
    
    // Initialize 36 main inventory slots with mock data
    m_InventorySlots.resize(36);
    for (int i = 0; i < 36; ++i) {
        m_InventorySlots[i].name = "";
        m_InventorySlots[i].count = 0;
    }
    
    // Add some mock items for testing
    m_InventorySlots[0].name = "Stone";
    m_InventorySlots[0].count = 64;
    m_InventorySlots[1].name = "Wood";
    m_InventorySlots[1].count = 32;
    m_InventorySlots[5].name = "Diamond";
    m_InventorySlots[5].count = 5;

    // Initialize 9 hotbar slots with mock data
    m_HotbarSlots.resize(9);
    for (int i = 0; i < 9; ++i) {
        m_HotbarSlots[i].name = "";
        m_HotbarSlots[i].count = 0;
    }
    
    m_HotbarSlots[0].name = "Pickaxe";
    m_HotbarSlots[0].count = 1;
    m_HotbarSlots[1].name = "Sword";
    m_HotbarSlots[1].count = 1;
}

void InventoryUI::onEnter() {
    UIScreen::onEnter();
    m_Open = true;
}

void InventoryUI::onExit() {
    UIScreen::onExit();
    m_Open = false;
}

void InventoryUI::update(float deltaTime) {
    // No update logic needed for now
}

void InventoryUI::render() {
    if (!isActive() || !m_Open) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500.0f, 550.0f));
    ImGui::SetNextWindowBgAlpha(0.95f);

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | 
                                             ImGuiWindowFlags_NoResize | 
                                             ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Inventory", nullptr, windowFlags)) {
        ImGui::TextUnformatted("Inventory");
        ImGui::Separator();
        ImGui::Spacing();

        renderInventoryGrid();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        renderHotbar();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(-FLT_MIN, 0.0f))) {
            m_StateManager.popState();
        }
    }
    ImGui::End();
}

void InventoryUI::renderInventoryGrid() {
    ImGui::TextUnformatted("Main Inventory (9x4)");
    
    const float slotSize = 50.0f;
    const float spacing = 5.0f;
    
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 9; ++col) {
            int index = row * 9 + col;
            
            if (col > 0) {
                ImGui::SameLine(0.0f, spacing);
            }
            
            ImGui::PushID(index);
            drawSlot(m_InventorySlots[index], index, "inv");
            ImGui::PopID();
        }
    }
}

void InventoryUI::renderHotbar() {
    ImGui::TextUnformatted("Hotbar");
    
    const float slotSize = 50.0f;
    const float spacing = 5.0f;
    
    for (int i = 0; i < 9; ++i) {
        if (i > 0) {
            ImGui::SameLine(0.0f, spacing);
        }
        
        ImGui::PushID(100 + i);
        
        // Highlight selected hotbar slot
        if (i == m_SelectedHotbar) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        }
        
        drawSlot(m_HotbarSlots[i], 36 + i, "hotbar");
        
        if (i == m_SelectedHotbar) {
            ImGui::PopStyleColor(3);
        }
        
        ImGui::PopID();
    }
}

void InventoryUI::drawSlot(ItemSlot& slot, int globalIndex, const char* labelPrefix) {
    const float slotSize = 50.0f;
    
    std::string label = slot.name.empty() ? "Empty" : slot.name;
    if (slot.count > 0) {
        label += " (" + std::to_string(slot.count) + ")";
    }
    
    ImGui::Button(label.c_str(), ImVec2(slotSize, slotSize));
    
    // Drag and drop source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        m_DragSourceIndex = globalIndex;
        ImGui::SetDragDropPayload("INVENTORY_SLOT", &globalIndex, sizeof(int));
        ImGui::TextUnformatted(label.c_str());
        ImGui::EndDragDropSource();
    }
    
    // Drag and drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INVENTORY_SLOT")) {
            int sourceIndex = *(const int*)payload->Data;
            int targetIndex = globalIndex;
            
            // Swap items
            ItemSlot* sourceSlot = resolveSlot(sourceIndex);
            ItemSlot* targetSlot = resolveSlot(targetIndex);
            
            if (sourceSlot && targetSlot) {
                ItemSlot temp = *sourceSlot;
                *sourceSlot = *targetSlot;
                *targetSlot = temp;
                
                PC_INFO("InventoryUI: Swapped items at indices " + std::to_string(sourceIndex) + 
                       " and " + std::to_string(targetIndex));
            }
            
            m_DragSourceIndex = -1;
        }
        ImGui::EndDragDropTarget();
    }
}

InventoryUI::ItemSlot* InventoryUI::resolveSlot(int globalIndex) {
    if (globalIndex >= 0 && globalIndex < 36) {
        return &m_InventorySlots[globalIndex];
    } else if (globalIndex >= 36 && globalIndex < 45) {
        return &m_HotbarSlots[globalIndex - 36];
    }
    return nullptr;
}

void InventoryUI::toggle() {
    if (m_Open) {
        m_StateManager.popState();
    } else {
        m_StateManager.pushState(GameState::INVENTORY);
    }
}

bool InventoryUI::isOpen() const {
    return m_Open;
}

} // namespace PoorCraft

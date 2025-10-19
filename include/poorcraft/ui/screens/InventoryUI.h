#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

#include <vector>
#include <string>

namespace PoorCraft {

class GameStateManager;
using Config = ::poorcraft::Config;

class InventoryUI : public UIScreen {
public:
    InventoryUI(Config& config, GameStateManager& stateManager);

    void onEnter() override;
    void onExit() override;
    void update(float deltaTime) override;
    void render() override;

    void toggle();
    bool isOpen() const;

private:
    struct ItemSlot {
        std::string name;
        int count;
    };

    void renderInventoryGrid();
    void renderHotbar();
    void drawSlot(ItemSlot& slot, int globalIndex, const char* labelPrefix);
    ItemSlot* resolveSlot(int globalIndex);

    Config& m_Config;
    GameStateManager& m_StateManager;
    std::vector<ItemSlot> m_InventorySlots;
    std::vector<ItemSlot> m_HotbarSlots;
    bool m_Open = false;
    int m_SelectedHotbar = 0;
    int m_DragSourceIndex = -1;
};

} // namespace PoorCraft

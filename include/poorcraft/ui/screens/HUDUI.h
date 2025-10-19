#pragma once

#include "poorcraft/ui/UIScreen.h"
#include "poorcraft/core/Config.h"

namespace PoorCraft {

class Player;
class World;
class GameLoop;
using Config = ::poorcraft::Config;

class HUDUI : public UIScreen {
public:
    explicit HUDUI(Config& config);

    void bindContext(Player* player, World* world, GameLoop* gameLoop);

    void render() override;

    void setShowDebug(bool value);
    bool isDebugVisible() const;

private:
    void drawCrosshair();
    void drawHealthBar();
    void drawHungerBar();
    void drawHotbar();
    void drawDebugInfo();
    void drawCoordinates();

    Player* m_Player = nullptr;
    World* m_World = nullptr;
    GameLoop* m_GameLoop = nullptr;
    Config& m_Config;

    bool m_ShowDebug = false;
    float m_HudOpacity = 0.8f;
};

} // namespace PoorCraft

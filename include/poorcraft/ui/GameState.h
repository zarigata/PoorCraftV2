#pragma once

#include <functional>
#include <string>
#include <vector>

namespace PoorCraft {

enum class GameState {
    MAIN_MENU,
    SINGLEPLAYER_LOADING,
    MULTIPLAYER_MENU,
    SERVER_BROWSER,
    CONNECTING,
    IN_GAME,
    PAUSED,
    SETTINGS,
    INVENTORY,
    CHAT
};

class GameStateManager {
public:
    using StateChangeCallback = std::function<void(GameState, GameState)>;

    static GameStateManager& getInstance();

    void initialize();

    void pushState(GameState state);
    void popState();
    void setState(GameState state);

    GameState getCurrentState() const;
    GameState getPreviousState() const;

    bool isInGame() const;
    bool isPaused() const;
    bool isInMenu() const;

    std::string getStateName(GameState state) const;

    std::size_t registerStateChangeCallback(StateChangeCallback callback);

private:
    GameStateManager() = default;

    void notifyStateChange(GameState oldState, GameState newState);

    std::vector<GameState> m_StateStack;
    std::vector<StateChangeCallback> m_Callbacks;
};

} // namespace PoorCraft

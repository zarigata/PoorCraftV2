#include "poorcraft/ui/GameState.h"

#include "poorcraft/core/Logger.h"

namespace PoorCraft {

GameStateManager& GameStateManager::getInstance() {
    static GameStateManager instance;
    return instance;
}

void GameStateManager::initialize() {
    if (!m_StateStack.empty()) {
        PC_WARN("GameStateManager already initialized");
        return;
    }

    m_StateStack.push_back(GameState::MAIN_MENU);
    PC_INFO("GameState initialized: MainMenu");
}

void GameStateManager::pushState(GameState state) {
    GameState previous = getCurrentState();
    m_StateStack.push_back(state);
    PC_INFO("GameState push: " + getStateName(previous) + " -> " + getStateName(state));
    notifyStateChange(previous, state);
}

void GameStateManager::popState() {
    if (m_StateStack.size() <= 1) {
        PC_WARN("Attempted to pop the last game state");
        return;
    }

    GameState oldState = m_StateStack.back();
    m_StateStack.pop_back();
    GameState newState = m_StateStack.back();
    PC_INFO("GameState pop: " + getStateName(oldState) + " -> " + getStateName(newState));
    notifyStateChange(oldState, newState);
}

void GameStateManager::setState(GameState state) {
    if (m_StateStack.empty()) {
        m_StateStack.push_back(state);
        PC_INFO("GameState set: " + getStateName(state));
        notifyStateChange(state, state);
        return;
    }

    GameState oldState = m_StateStack.back();
    m_StateStack.clear();
    m_StateStack.push_back(state);
    PC_INFO("GameState set: " + getStateName(oldState) + " -> " + getStateName(state));
    notifyStateChange(oldState, state);
}

GameState GameStateManager::getCurrentState() const {
    if (m_StateStack.empty()) {
        return GameState::MAIN_MENU;
    }
    return m_StateStack.back();
}

GameState GameStateManager::getPreviousState() const {
    if (m_StateStack.size() < 2) {
        return getCurrentState();
    }
    return m_StateStack[m_StateStack.size() - 2];
}

bool GameStateManager::isInGame() const {
    return getCurrentState() == GameState::IN_GAME;
}

bool GameStateManager::isPaused() const {
    return getCurrentState() == GameState::PAUSED;
}

bool GameStateManager::isInMenu() const {
    GameState current = getCurrentState();
    return current != GameState::IN_GAME && current != GameState::CHAT && current != GameState::INVENTORY;
}

std::string GameStateManager::getStateName(GameState state) const {
    switch (state) {
        case GameState::MAIN_MENU: return "MainMenu";
        case GameState::SINGLEPLAYER_LOADING: return "SingleplayerLoading";
        case GameState::MULTIPLAYER_MENU: return "MultiplayerMenu";
        case GameState::SERVER_BROWSER: return "ServerBrowser";
        case GameState::CONNECTING: return "Connecting";
        case GameState::IN_GAME: return "InGame";
        case GameState::PAUSED: return "Paused";
        case GameState::SETTINGS: return "Settings";
        case GameState::INVENTORY: return "Inventory";
        case GameState::CHAT: return "Chat";
        default: return "Unknown";
    }
}

std::size_t GameStateManager::registerStateChangeCallback(StateChangeCallback callback) {
    m_Callbacks.push_back(std::move(callback));
    return m_Callbacks.size() - 1;
}

void GameStateManager::notifyStateChange(GameState oldState, GameState newState) {
    for (const auto& callback : m_Callbacks) {
        if (callback) {
            callback(oldState, newState);
        }
    }
}

} // namespace PoorCraft

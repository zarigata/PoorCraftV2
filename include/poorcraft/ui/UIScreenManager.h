#pragma once

#include <memory>
#include <string>
#include <vector>

#include "poorcraft/ui/GameState.h"

namespace poorcraft {
class Config;
}

namespace PoorCraft {

class UIScreen;
class Window;
class GameLoop;
class Renderer;
class Camera;
class NetworkManager;
class MainMenuUI;
class PauseMenuUI;
class SettingsUI;
class ServerBrowserUI;
class HUDUI;
class ChatUI;
class InventoryUI;
class MultiplayerMenuUI;
class LoadingScreenUI;
class World;
class PhysicsWorld;
class PlayerController;
class Transform;

class UIScreenManager {
public:
    struct GameSession {
        std::unique_ptr<World> world;
        std::shared_ptr<PhysicsWorld> physicsWorld;
        std::unique_ptr<class AnimationSystem> animationSystem;
        std::unique_ptr<class EntityRenderer> entityRenderer;
        class Entity* playerEntity = nullptr;
        Transform* playerTransform = nullptr;
        PlayerController* playerController = nullptr;
    };

    static UIScreenManager& getInstance();

    void initialize(Window& window,
                    poorcraft::Config& config,
                    NetworkManager& networkManager,
                    Renderer& renderer,
                    GameLoop& gameLoop,
                    Camera& camera);
    void shutdown();

    void update(float deltaTime);
    void render();

    GameSession* getGameSession();
    bool isGameplayReady() const;

    HUDUI* getHUD() const;
    ChatUI* getChat() const;
    InventoryUI* getInventory() const;

    void notifyChatMessage(const std::string& sender, const std::string& message, bool system);
    void setLoadingProgress(float progress, const std::string& tip);

    bool shouldCloseApplication() const;
    void resetCloseRequest();
    void requestCloseApplication();

private:
    UIScreenManager() = default;

    void activateScreensForState(GameState state);
    void handleStateTransition(GameState oldState, GameState newState);
    void setScreenActive(UIScreen& screen, bool active);
    void deactivateAllScreens();

    void startSingleplayerLoading();
    void updateSingleplayerLoading(float deltaTime);
    void enterInGame();
    void shutdownGameSession();

    void updateCursorVisibility(GameState state);

    Window* m_Window = nullptr;
    poorcraft::Config* m_Config = nullptr;
    NetworkManager* m_NetworkManager = nullptr;
    Renderer* m_Renderer = nullptr;
    GameLoop* m_GameLoop = nullptr;
    Camera* m_Camera = nullptr;

    std::unique_ptr<GameSession> m_GameSession;

    std::unique_ptr<MainMenuUI> m_MainMenu;
    std::unique_ptr<PauseMenuUI> m_PauseMenu;
    std::unique_ptr<SettingsUI> m_Settings;
    std::unique_ptr<ServerBrowserUI> m_ServerBrowser;
    std::unique_ptr<HUDUI> m_HUD;
    std::unique_ptr<ChatUI> m_Chat;
    std::unique_ptr<InventoryUI> m_Inventory;
    std::unique_ptr<MultiplayerMenuUI> m_MultiplayerMenu;
    std::unique_ptr<LoadingScreenUI> m_LoadingScreen;

    std::vector<UIScreen*> m_ActiveScreens;

    std::size_t m_StateCallbackId = 0;
    float m_LoadProgress = 0.0f;
    std::string m_LoadTip;
    bool m_CloseRequested = false;
    bool m_Initialized = false;
};

} // namespace PoorCraft

#include "poorcraft/ui/UIScreenManager.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/core/GameLoop.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/entity/Entity.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/entity/HumanoidMesh.h"
#include "poorcraft/entity/PlayerSkin.h"
#include "poorcraft/entity/components/AnimationController.h"
#include "poorcraft/entity/components/PlayerController.h"
#include "poorcraft/entity/components/Renderable.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/entity/systems/AnimationSystem.h"
#include "poorcraft/entity/systems/EntityRenderer.h"
#include "poorcraft/input/Input.h"
#include "poorcraft/network/NetworkManager.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/Texture.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/ui/GameState.h"
#include "poorcraft/ui/UIManager.h"
#include "poorcraft/ui/screens/ChatUI.h"
#include "poorcraft/ui/screens/HUDUI.h"
#include "poorcraft/ui/screens/InventoryUI.h"
#include "poorcraft/ui/screens/LoadingScreenUI.h"
#include "poorcraft/ui/screens/MainMenuUI.h"
#include "poorcraft/ui/screens/MultiplayerMenuUI.h"
#include "poorcraft/ui/screens/PauseMenuUI.h"
#include "poorcraft/ui/screens/ServerBrowserUI.h"
#include "poorcraft/ui/screens/SettingsUI.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/world/World.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace PoorCraft {

namespace {
constexpr float LOADING_COMPLETE_THRESHOLD = 0.99f;
constexpr float MIN_LOADING_DURATION = 0.75f;
constexpr int DEFAULT_PLAYER_START_HEIGHT = 70;
constexpr int CHAT_OVERLAY_RECENT_MESSAGES = 5;

std::shared_ptr<Texture> resolvePlayerTexture(ResourceHandle<PlayerSkin>& skinHandle, Renderer& renderer) {
    if (skinHandle && skinHandle->getTexture()) {
        return skinHandle->getTexture();
    }
    return renderer.getDefaultTexture();
}

} // namespace

UIScreenManager& UIScreenManager::getInstance() {
    static UIScreenManager instance;
    return instance;
}

void UIScreenManager::initialize(Window& window,
                                 poorcraft::Config& config,
                                 NetworkManager& networkManager,
                                 Renderer& renderer,
                                 GameLoop& gameLoop,
                                 Camera& camera) {
    if (m_Initialized) {
        PC_WARN("UIScreenManager already initialized");
        return;
    }

    m_Window = &window;
    m_Config = &config;
    m_NetworkManager = &networkManager;
    m_Renderer = &renderer;
    m_GameLoop = &gameLoop;
    m_Camera = &camera;

    UIManager::getInstance().initialize(window);

    auto& stateManager = GameStateManager::getInstance();

    m_MainMenu = std::make_unique<MainMenuUI>(stateManager, networkManager, window);
    m_PauseMenu = std::make_unique<PauseMenuUI>(stateManager, networkManager);
    m_Settings = std::make_unique<SettingsUI>(config, window, renderer, stateManager);
    m_ServerBrowser = std::make_unique<ServerBrowserUI>(stateManager, networkManager, config);
    m_HUD = std::make_unique<HUDUI>(config);
    m_Chat = std::make_unique<ChatUI>(networkManager, config, stateManager);
    m_Inventory = std::make_unique<InventoryUI>(config, stateManager);
    m_MultiplayerMenu = std::make_unique<MultiplayerMenuUI>(stateManager, networkManager, config);
    m_LoadingScreen = std::make_unique<LoadingScreenUI>(stateManager, config);

    m_StateCallbackId = stateManager.registerStateChangeCallback([this](GameState oldState, GameState newState) {
        handleStateTransition(oldState, newState);
    });

    activateScreensForState(stateManager.getCurrentState());

    m_Initialized = true;
}

void UIScreenManager::shutdown() {
    if (!m_Initialized) {
        return;
    }

    deactivateAllScreens();
    shutdownGameSession();

    UIManager::getInstance().shutdown();

    m_MainMenu.reset();
    m_PauseMenu.reset();
    m_Settings.reset();
    m_ServerBrowser.reset();
    m_HUD.reset();
    m_Chat.reset();
    m_Inventory.reset();
    m_MultiplayerMenu.reset();
    m_LoadingScreen.reset();

    m_Window = nullptr;
    m_Config = nullptr;
    m_NetworkManager = nullptr;
    m_Renderer = nullptr;
    m_GameLoop = nullptr;
    m_Camera = nullptr;

    m_Initialized = false;
}

void UIScreenManager::update(float deltaTime) {
    if (!m_Initialized) {
        return;
    }

    UIManager::getInstance().beginFrame();

    GameStateManager& stateManager = GameStateManager::getInstance();
    GameState currentState = stateManager.getCurrentState();
    activateScreensForState(currentState);

    ImGuiIO& io = ImGui::GetIO();
    bool wantKeyboard = io.WantCaptureKeyboard;

    auto& input = Input::getInstance();

    if (!wantKeyboard) {
        if (currentState == GameState::IN_GAME) {
            if (input.wasKeyJustPressed(GLFW_KEY_ESCAPE)) {
                stateManager.pushState(GameState::PAUSED);
            }
            if (input.wasKeyJustPressed(GLFW_KEY_E)) {
                stateManager.pushState(GameState::INVENTORY);
            }
        } else if (currentState == GameState::PAUSED) {
            if (input.wasKeyJustPressed(GLFW_KEY_ESCAPE)) {
                stateManager.popState();
            }
        } else if (currentState == GameState::INVENTORY) {
            if (input.wasKeyJustPressed(GLFW_KEY_ESCAPE) || input.wasKeyJustPressed(GLFW_KEY_E)) {
                stateManager.popState();
            }
        } else if (currentState == GameState::CHAT) {
            if (input.wasKeyJustPressed(GLFW_KEY_ESCAPE) || input.wasKeyJustPressed(GLFW_KEY_ENTER)) {
                if (m_Chat) {
                    m_Chat->toggleChat();
                }
                stateManager.popState();
            }
        }

        if (currentState == GameState::IN_GAME || currentState == GameState::CHAT) {
            if (input.wasKeyJustPressed(GLFW_KEY_T)) {
                if (m_Chat) {
                    bool wasOpen = m_Chat->isChatOpen();
                    m_Chat->toggleChat();
                    if (m_Chat->isChatOpen() && !wasOpen) {
                        stateManager.pushState(GameState::CHAT);
                    } else if (wasOpen && !m_Chat->isChatOpen() && stateManager.getCurrentState() == GameState::CHAT) {
                        stateManager.popState();
                    }
                }
            }
        }
    }

    if (currentState == GameState::SINGLEPLAYER_LOADING) {
        updateSingleplayerLoading(deltaTime);
    }

    for (UIScreen* screen : m_ActiveScreens) {
        if (screen && screen->isActive()) {
            screen->update(deltaTime);
        }
    }

    if (m_CloseRequested && m_Window) {
        if (GLFWwindow* native = m_Window->getNativeWindow()) {
            glfwSetWindowShouldClose(native, GLFW_TRUE);
        }
    }
}

void UIScreenManager::render() {
    if (!m_Initialized) {
        return;
    }

    for (UIScreen* screen : m_ActiveScreens) {
        if (screen && screen->isActive()) {
            screen->render();
        }
    }

    UIManager::getInstance().endFrame();
}

UIScreenManager::GameSession* UIScreenManager::getGameSession() {
    return m_GameSession.get();
}

bool UIScreenManager::isGameplayReady() const {
    return m_GameplayReady;
}

HUDUI* UIScreenManager::getHUD() const {
    return m_HUD.get();
}

ChatUI* UIScreenManager::getChat() const {
    return m_Chat.get();
}

InventoryUI* UIScreenManager::getInventory() const {
    return m_Inventory.get();
}

void UIScreenManager::notifyChatMessage(const std::string& sender, const std::string& message, bool system) {
    if (m_Chat) {
        m_Chat->addMessage(sender, message, system);
    }
}

void UIScreenManager::setLoadingProgress(float progress, const std::string& tip) {
    m_LoadProgress = std::clamp(progress, 0.0f, 1.0f);
    m_LoadTip = tip;
    if (m_LoadingScreen) {
        m_LoadingScreen->setProgress(m_LoadProgress, m_LoadTip);
    }
}

bool UIScreenManager::shouldCloseApplication() const {
    return m_CloseRequested;
}

void UIScreenManager::resetCloseRequest() {
    m_CloseRequested = false;
}

void UIScreenManager::requestCloseApplication() {
    m_CloseRequested = true;
    if (m_Window) {
        if (GLFWwindow* native = m_Window->getNativeWindow()) {
            glfwSetWindowShouldClose(native, GLFW_TRUE);
        }
    }
}

void UIScreenManager::activateScreensForState(GameState state) {
    if (!m_Initialized) {
        return;
    }

    deactivateAllScreens();

    switch (state) {
        case GameState::MAIN_MENU:
            if (m_MainMenu) {
                setScreenActive(*m_MainMenu, true);
            }
            break;
        case GameState::MULTIPLAYER_MENU:
            if (m_MultiplayerMenu) {
                setScreenActive(*m_MultiplayerMenu, true);
            }
            break;
        case GameState::SERVER_BROWSER:
            if (m_ServerBrowser) {
                setScreenActive(*m_ServerBrowser, true);
            }
            break;
        case GameState::SINGLEPLAYER_LOADING:
            if (m_LoadingScreen) {
                if (m_GameSession && m_LoadingScreen) {
                    m_LoadingScreen->bindWorld(m_GameSession->world.get());
                }
                setScreenActive(*m_LoadingScreen, true);
            }
            break;
        case GameState::IN_GAME:
            if (m_HUD) {
                setScreenActive(*m_HUD, true);
            }
            if (m_Chat) {
                setScreenActive(*m_Chat, true);
            }
            break;
        case GameState::PAUSED:
            if (m_HUD) {
                setScreenActive(*m_HUD, true);
            }
            if (m_PauseMenu) {
                setScreenActive(*m_PauseMenu, true);
            }
            break;
        case GameState::SETTINGS:
            if (m_Settings) {
                setScreenActive(*m_Settings, true);
            }
            break;
        case GameState::INVENTORY:
            if (m_HUD) {
                setScreenActive(*m_HUD, true);
            }
            if (m_Inventory) {
                setScreenActive(*m_Inventory, true);
            }
            break;
        case GameState::CHAT:
            if (m_HUD) {
                setScreenActive(*m_HUD, true);
            }
            if (m_Chat) {
                setScreenActive(*m_Chat, true);
            }
            break;
        default:
            break;
    }
}

void UIScreenManager::handleStateTransition(GameState oldState, GameState newState) {
    updateCursorVisibility(newState);

    switch (newState) {
        case GameState::MAIN_MENU:
            if (m_NetworkManager && m_NetworkManager->isClient()) {
                m_NetworkManager->disconnect();
            }
            shutdownGameSession();
            break;
        case GameState::SINGLEPLAYER_LOADING:
            startSingleplayerLoading();
            break;
        case GameState::IN_GAME:
            if (m_PendingEnterGame) {
                enterInGame();
            }
            m_GameplayReady = true;
            UIManager::getInstance().setMouseCursor(false);
            break;
        case GameState::PAUSED:
        case GameState::SETTINGS:
        case GameState::MULTIPLAYER_MENU:
        case GameState::SERVER_BROWSER:
        case GameState::INVENTORY:
        case GameState::CHAT:
        case GameState::SINGLEPLAYER_LOADING:
            UIManager::getInstance().setMouseCursor(true);
            break;
        default:
            break;
    }

    if (oldState == GameState::IN_GAME && newState != GameState::IN_GAME) {
        UIManager::getInstance().setMouseCursor(true);
    }
}

void UIScreenManager::setScreenActive(UIScreen& screen, bool active) {
    const bool currentlyActive = screen.isActive();
    if (active) {
        if (!currentlyActive) {
            screen.onEnter();
        }
        if (std::find(m_ActiveScreens.begin(), m_ActiveScreens.end(), &screen) == m_ActiveScreens.end()) {
            m_ActiveScreens.push_back(&screen);
        }
    } else {
        if (currentlyActive) {
            screen.onExit();
        }
        m_ActiveScreens.erase(std::remove(m_ActiveScreens.begin(), m_ActiveScreens.end(), &screen), m_ActiveScreens.end());
    }
}

void UIScreenManager::deactivateAllScreens() {
    for (UIScreen* screen : m_ActiveScreens) {
        if (screen && screen->isActive()) {
            screen->onExit();
        }
    }
    m_ActiveScreens.clear();
}

void UIScreenManager::startSingleplayerLoading() {
    if (!m_Config) {
        PC_ERROR("UIScreenManager: Configuration unavailable for loading session");
        return;
    }

    m_GameSession = std::make_unique<GameSession>();
    m_LoadElapsed = 0.0f;
    m_LoadProgress = 0.0f;
    m_PendingEnterGame = false;
    m_GameplayReady = false;

    m_GameSession->world = std::make_unique<World>();
    int renderDistance = m_Config->get_int(poorcraft::Config::GameplayConfig::RENDER_DISTANCE_KEY, 8);

    if (!m_GameSession->world->initialize(renderDistance)) {
        PC_ERROR("UIScreenManager: World initialization failed");
        m_GameSession.reset();
        GameStateManager::getInstance().setState(GameState::MAIN_MENU);
        return;
    }

    m_GameSession->physicsWorld = std::make_shared<PhysicsWorld>(*m_GameSession->world);
    m_GameSession->animationSystem = std::make_unique<AnimationSystem>(EntityManager::getInstance());
    m_GameSession->entityRenderer = std::make_unique<EntityRenderer>(EntityManager::getInstance());

    if (m_NetworkManager) {
        m_NetworkManager->setWorld(m_GameSession->world.get());
        m_NetworkManager->setEntityManager(&EntityManager::getInstance());
    }

    if (m_LoadingScreen) {
        m_LoadingScreen->bindWorld(m_GameSession->world.get());
    }

    setLoadingProgress(0.1f, "Preparing terrain...");
    m_PendingEnterGame = true;
}

void UIScreenManager::updateSingleplayerLoading(float deltaTime) {
    if (!m_GameSession) {
        return;
    }

    m_LoadElapsed += deltaTime;

    float progress = std::min(LOADING_COMPLETE_THRESHOLD, m_LoadElapsed / MIN_LOADING_DURATION);
    setLoadingProgress(progress, "Generating world...");

    if (progress >= LOADING_COMPLETE_THRESHOLD && m_PendingEnterGame) {
        GameStateManager::getInstance().setState(GameState::IN_GAME);
    }
}

void UIScreenManager::enterInGame() {
    if (!m_GameSession || !m_Renderer || !m_Camera) {
        PC_ERROR("UIScreenManager: Cannot enter game due to missing dependencies");
        return;
    }

    auto& entityManager = EntityManager::getInstance();
    auto& resourceManager = ResourceManager::getInstance();

    auto playerSkinHandle = resourceManager.load<PlayerSkin>("assets/skins/player.png");
    auto playerMeshData = HumanoidMesh::generate(playerSkinHandle.isValid() ? playerSkinHandle.get() : nullptr);
    if (!playerMeshData.mesh) {
        PC_ERROR("UIScreenManager: Failed to generate player mesh");
        GameStateManager::getInstance().setState(GameState::MAIN_MENU);
        return;
    }

    std::shared_ptr<Texture> playerTexture = resolvePlayerTexture(playerSkinHandle, *m_Renderer);

    Entity& playerEntity = entityManager.createEntity("Player");
    Transform& transform = playerEntity.addComponent<Transform>();
    transform.setPosition(glm::vec3(0.0f, static_cast<float>(DEFAULT_PLAYER_START_HEIGHT), 0.0f));
    transform.updatePrevious();

    std::shared_ptr<Camera> cameraShared(m_Camera, [](Camera*) {});

    if (!m_GameSession->physicsWorld) {
        m_GameSession->physicsWorld = std::make_shared<PhysicsWorld>(*m_GameSession->world);
    }

    PlayerController& controller = playerEntity.addComponent<PlayerController>(m_GameSession->physicsWorld, cameraShared);
    controller.getPlayer().setPosition(transform.getPosition());

    std::vector<Renderable::Section> playerSections;
    playerSections.reserve(playerMeshData.sections.size());
    for (const auto& section : playerMeshData.sections) {
        playerSections.push_back({section.indexOffset, section.indexCount});
    }

    Renderable& renderable = playerEntity.addComponent<Renderable>(playerMeshData.mesh, playerTexture, std::move(playerSections));
    renderable.setRenderLayer(0);

    playerEntity.addComponent<AnimationController>();

    m_GameSession->playerEntity = &playerEntity;
    m_GameSession->playerTransform = &transform;
    m_GameSession->playerController = &controller;

    if (m_HUD) {
        m_HUD->bindContext(&controller.getPlayer(), m_GameSession->world.get(), m_GameLoop);
    }

    if (m_Chat) {
        m_Chat->onEnter();
    }

    m_Renderer->setCamera(m_Camera);

    m_PendingEnterGame = false;
    setLoadingProgress(1.0f, "World ready");
}

void UIScreenManager::shutdownGameSession() {
    if (!m_GameSession) {
        return;
    }

    if (m_NetworkManager) {
        m_NetworkManager->setWorld(nullptr);
        m_NetworkManager->setEntityManager(nullptr);
    }

    if (m_GameSession->playerEntity) {
        EntityManager::getInstance().destroyEntity(m_GameSession->playerEntity->getId());
        m_GameSession->playerEntity = nullptr;
        m_GameSession->playerTransform = nullptr;
        m_GameSession->playerController = nullptr;
    }

    if (m_GameSession->world) {
        m_GameSession->world->shutdown();
    }

    m_GameSession.reset();
    m_GameplayReady = false;
    m_PendingEnterGame = false;
    m_LoadProgress = 0.0f;
    m_LoadTip.clear();
}

void UIScreenManager::updateCursorVisibility(GameState state) {
    if (!m_Initialized) {
        return;
    }

    bool showCursor = state != GameState::IN_GAME;
    UIManager::getInstance().setMouseCursor(showCursor);
}

} // namespace PoorCraft

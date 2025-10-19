#include "poorcraft/core/Logger.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/platform/Platform.h"
#include "poorcraft/window/Window.h"
#include "poorcraft/input/Input.h"
#include "poorcraft/core/GameLoop.h"
#include "poorcraft/core/EventBus.h"
#include "poorcraft/resource/ResourceManager.h"
#include "poorcraft/resource/BinaryResource.h"
#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Shader.h"
#include "poorcraft/rendering/Texture.h"
#include "poorcraft/world/World.h"
#include "poorcraft/entity/EntityManager.h"
#include "poorcraft/entity/components/Transform.h"
#include "poorcraft/entity/components/PlayerController.h"
#include "poorcraft/entity/components/Renderable.h"
#include "poorcraft/entity/components/AnimationController.h"
#include "poorcraft/entity/systems/AnimationSystem.h"
#include "poorcraft/entity/systems/EntityRenderer.h"
#include "poorcraft/entity/HumanoidMesh.h"
#include "poorcraft/entity/PlayerSkin.h"
#include "poorcraft/physics/Player.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <cmath>
#if defined(__has_include)
#if __has_include("poorcraft/rendering/DebugRenderer.h")
#include "poorcraft/rendering/DebugRenderer.h"
#define POORCRAFT_HAS_DEBUG_RENDERER 1
#endif
#endif
#ifndef POORCRAFT_HAS_DEBUG_RENDERER
#define POORCRAFT_HAS_DEBUG_RENDERER 0
#endif
#include "poorcraft/events/WindowEvent.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
        // Initialize logging system
        poorcraft::Logger::get_instance().initialize(poorcraft::LogLevel::INFO, true, "poorcraft.log");

        // Log engine startup
        PC_INFO("=== PoorCraft Engine v0.1.0 ===");
        PC_INFO("Starting PoorCraft game engine...");
        PC_INFO("Platform: " + poorcraft::Platform::get_platform_name());
        PC_INFO("Working Directory: " + poorcraft::Platform::get_current_working_directory());
        PC_INFO("Executable Path: " + poorcraft::Platform::get_executable_path());

        // Initialize configuration system
        PC_INFO("Loading configuration...");
        poorcraft::Config& config = poorcraft::Config::get_instance();

        // Set default configuration if config file doesn't exist or is empty
        if (!poorcraft::Platform::file_exists("config.ini")) {
            PC_INFO("Creating default configuration file...");

            // Graphics settings
            config.set_int(poorcraft::Config::GraphicsConfig::WIDTH_KEY, 1280);
            config.set_int(poorcraft::Config::GraphicsConfig::HEIGHT_KEY, 720);
            config.set_bool(poorcraft::Config::GraphicsConfig::FULLSCREEN_KEY, false);
            config.set_bool(poorcraft::Config::GraphicsConfig::VSYNC_KEY, true);
            config.set_int(poorcraft::Config::GraphicsConfig::FOV_KEY, 90);

            // Audio settings
            config.set_float(poorcraft::Config::AudioConfig::MASTER_VOLUME_KEY, 1.0f);
            config.set_float(poorcraft::Config::AudioConfig::MUSIC_VOLUME_KEY, 0.7f);
            config.set_float(poorcraft::Config::AudioConfig::SOUND_VOLUME_KEY, 0.8f);

            // Controls settings
            config.set_float(poorcraft::Config::ControlsConfig::MOUSE_SENSITIVITY_KEY, 1.0f);
            config.set_bool(poorcraft::Config::ControlsConfig::INVERT_Y_KEY, false);

            // Gameplay settings
            config.set_int(poorcraft::Config::GameplayConfig::RENDER_DISTANCE_KEY, 8);
            config.set_string(poorcraft::Config::GameplayConfig::DIFFICULTY_KEY, "normal");

            // Network settings
            config.set_int(poorcraft::Config::NetworkConfig::DEFAULT_PORT_KEY, 25565);
            config.set_int(poorcraft::Config::NetworkConfig::TIMEOUT_KEY, 5000);

            // Engine settings
            config.set_string(poorcraft::Config::EngineConfig::LOG_LEVEL_KEY, "info");
            config.set_int(poorcraft::Config::EngineConfig::MAX_FPS_KEY, 144);

            // Save default configuration
            if (!config.save_to_file("config.ini")) {
                PC_WARN("Failed to save default configuration file");
            }
        } else {
            // Load existing configuration
            if (!config.load_from_file("config.ini")) {
                PC_WARN("Failed to load configuration file, using defaults");
            }
        }

        // Update logger level based on configuration
        std::string log_level_str = config.get_string(poorcraft::Config::EngineConfig::LOG_LEVEL_KEY, "info");
        poorcraft::LogLevel log_level = poorcraft::string_to_log_level(log_level_str);
        poorcraft::Logger::get_instance().set_log_level(log_level);

        // Log loaded configuration values
        PC_INFO("Configuration loaded:");
        PC_INFO("  Graphics: " + std::to_string(config.get_int(poorcraft::Config::GraphicsConfig::WIDTH_KEY)) +
                "x" + std::to_string(config.get_int(poorcraft::Config::GraphicsConfig::HEIGHT_KEY)) +
                (config.get_bool(poorcraft::Config::GraphicsConfig::FULLSCREEN_KEY) ? " (fullscreen)" : " (windowed)"));
        PC_INFO("  Audio: Master=" + std::to_string(config.get_float(poorcraft::Config::AudioConfig::MASTER_VOLUME_KEY)) +
                ", Music=" + std::to_string(config.get_float(poorcraft::Config::AudioConfig::MUSIC_VOLUME_KEY)) +
                ", Sound=" + std::to_string(config.get_float(poorcraft::Config::AudioConfig::SOUND_VOLUME_KEY)));
        PC_INFO("  Controls: Sensitivity=" + std::to_string(config.get_float(poorcraft::Config::ControlsConfig::MOUSE_SENSITIVITY_KEY)) +
                (config.get_bool(poorcraft::Config::ControlsConfig::INVERT_Y_KEY) ? " (inverted)" : " (normal)"));
        PC_INFO("  Gameplay: Render Distance=" + std::to_string(config.get_int(poorcraft::Config::GameplayConfig::RENDER_DISTANCE_KEY)) +
                ", Difficulty=" + config.get_string(poorcraft::Config::GameplayConfig::DIFFICULTY_KEY));
        PC_INFO("  Network: Port=" + std::to_string(config.get_int(poorcraft::Config::NetworkConfig::DEFAULT_PORT_KEY)) +
                ", Timeout=" + std::to_string(config.get_int(poorcraft::Config::NetworkConfig::TIMEOUT_KEY)));
        PC_INFO("  Engine: Max FPS=" + std::to_string(config.get_int(poorcraft::Config::EngineConfig::MAX_FPS_KEY)));

        // Initialize GLFW
        PC_INFO("=== Initializing Window System ===");
        if (!PoorCraft::Window::initializeGLFW()) {
            PC_FATAL("Failed to initialize GLFW");
            return 1;
        }

        // Query and log available monitors
        auto monitors = PoorCraft::Window::getMonitors();
        PC_INFO("Detected " + std::to_string(monitors.size()) + " monitor(s)");
        for (const auto& monitor : monitors) {
            PC_INFO("  Monitor " + std::to_string(monitor.id) + ": " + monitor.name + 
                    " (" + std::to_string(monitor.width) + "x" + std::to_string(monitor.height) + 
                    " @ " + std::to_string(monitor.refreshRate) + "Hz)");
        }

        // Create window properties from config
        PoorCraft::WindowProperties windowProps;
        windowProps.title = "PoorCraft v0.1.0";
        windowProps.width = config.get_int(poorcraft::Config::GraphicsConfig::WIDTH_KEY, 1280);
        windowProps.height = config.get_int(poorcraft::Config::GraphicsConfig::HEIGHT_KEY, 720);
        windowProps.fullscreen = config.get_bool(poorcraft::Config::GraphicsConfig::FULLSCREEN_KEY, false);
        windowProps.vsync = config.get_bool(poorcraft::Config::GraphicsConfig::VSYNC_KEY, true);

        // Create and initialize window
        PoorCraft::Window window(windowProps);
        if (!window.initialize()) {
            PC_FATAL("Failed to initialize window");
            PoorCraft::Window::terminateGLFW();
            return 1;
        }

        // Set window event callback to forward events to EventBus
        window.setEventCallback([](PoorCraft::Event& e) {
            PoorCraft::EventBus::getInstance().publish(e);
        });

        // Forward all events to the input system
        [[maybe_unused]] const size_t inputSubscription = PoorCraft::EventBus::getInstance().subscribe(
            PoorCraft::EventType::None,
            [](PoorCraft::Event& e) {
                PoorCraft::Input::getInstance().onEvent(e);
            });

        // Subscribe to window close event
        PoorCraft::EventBus::getInstance().subscribe(PoorCraft::EventType::WindowClose, 
            [](PoorCraft::Event& e) {
                PC_INFO("Window close requested by user");
            });

        // Initialize Renderer after window creation
        PC_INFO("=== Initializing Renderer ===");
        if (!PoorCraft::Renderer::getInstance().initialize()) {
            PC_FATAL("Failed to initialize renderer");
            PoorCraft::Window::terminateGLFW();
            return 1;
        }

        // Create camera with perspective projection
        PC_INFO("=== Creating Camera ===");
        float fov = config.get_int(poorcraft::Config::GraphicsConfig::FOV_KEY, 90);
        float aspectRatio = static_cast<float>(windowProps.width) / static_cast<float>(windowProps.height);
        PoorCraft::Camera camera(PoorCraft::CameraType::PERSPECTIVE, 
                                glm::vec3(0.0f, 0.0f, 5.0f), 
                                glm::vec3(0.0f, 0.0f, 0.0f));
        camera.updateProjectionMatrix(fov, aspectRatio, 0.1f, 1000.0f);
        PoorCraft::Renderer::getInstance().setCamera(&camera);

#if POORCRAFT_HAS_DEBUG_RENDERER
        PoorCraft::DebugRenderer::getInstance().initialize();
#endif

        // Subscribe to window resize events to update camera projection
        PoorCraft::EventBus::getInstance().subscribe(PoorCraft::EventType::WindowResize, 
            [&](PoorCraft::Event& e) {
                auto& resizeEvent = static_cast<PoorCraft::events::WindowResizeEvent&>(e);
                camera.onWindowResize(resizeEvent.getWidth(), resizeEvent.getHeight());
            });

        // Initialize ResourceManager
        PC_INFO("=== Initializing Resource Manager ===");
        std::string exePath = poorcraft::Platform::get_executable_directory();
        PoorCraft::ResourceManager::getInstance().setBasePath(exePath);

        // Create a test file for resource loading
        std::string testResourcePath = poorcraft::Platform::join_path(exePath, "test_resource.dat");
        std::string testContent = "PoorCraft Test Resource Data";
        poorcraft::Platform::write_file_text(testResourcePath, testContent);

        // Test resource loading
        auto testResource = PoorCraft::ResourceManager::getInstance().load<PoorCraft::BinaryResource>("test_resource.dat");
        if (testResource.isValid()) {
            PC_INFO("Successfully loaded test resource (" + std::to_string(testResource->getSize()) + " bytes)");
        }

        // Create game loop
        PC_INFO("=== Initializing Game Loop ===");
        PoorCraft::GameLoop gameLoop(window);
        
        // Set fixed timestep (60 updates per second)
        gameLoop.setFixedTimestep(1.0f / 60.0f);
        
        // Set max FPS from config
        int maxFPS = config.get_int(poorcraft::Config::EngineConfig::MAX_FPS_KEY, 144);
        gameLoop.setMaxFPS(maxFPS);

        // Initialize World system
        PC_INFO("=== Initializing World System ===");
        int renderDistance = config.get_int(poorcraft::Config::GameplayConfig::RENDER_DISTANCE_KEY, 8);
        auto world = std::make_unique<PoorCraft::World>();
        if (!world->initialize(renderDistance)) {
            PC_FATAL("Failed to initialize world system");
            PoorCraft::Renderer::getInstance().shutdown();
            window.shutdown();
            PoorCraft::Window::terminateGLFW();
            return 1;
        }

        auto physicsWorld = std::make_shared<PoorCraft::PhysicsWorld>(*world);
        auto cameraPtr = std::shared_ptr<PoorCraft::Camera>(&camera, [](PoorCraft::Camera*) {});

        auto& entityManager = PoorCraft::EntityManager::getInstance();
        PoorCraft::AnimationSystem animationSystem(entityManager);
        PoorCraft::EntityRenderer entityRenderer(entityManager);

        auto playerSkinHandle = PoorCraft::ResourceManager::getInstance().load<PoorCraft::PlayerSkin>("assets/skins/player.png");
        if (!playerSkinHandle.isValid()) {
            PC_WARN("Failed to load player skin, using default texture");
        }

        auto playerMeshData = PoorCraft::HumanoidMesh::generate(playerSkinHandle.isValid() ? playerSkinHandle.get() : nullptr);
        if (!playerMeshData.mesh) {
            throw std::runtime_error("Failed to generate humanoid mesh for player entity");
        }

        std::shared_ptr<PoorCraft::Texture> playerTexture;
        if (playerSkinHandle && playerSkinHandle->getTexture()) {
            playerTexture = playerSkinHandle->getTexture();
        } else {
            playerTexture = PoorCraft::Renderer::getInstance().getDefaultTexture();
        }

        auto& playerEntity = entityManager.createEntity("Player");
        auto& playerTransform = playerEntity.addComponent<PoorCraft::Transform>();
        playerTransform.setPosition(glm::vec3(0.0f, 70.0f, 0.0f));
        playerTransform.updatePrevious();

        auto& playerControllerComponent = playerEntity.addComponent<PoorCraft::PlayerController>(physicsWorld, cameraPtr);
        playerControllerComponent.getPlayer().setPosition(playerTransform.getPosition());

        std::vector<PoorCraft::Renderable::Section> playerSections;
        playerSections.reserve(playerMeshData.sections.size());
        for (const auto& section : playerMeshData.sections) {
            playerSections.push_back({ section.indexOffset, section.indexCount });
        }

        auto& playerRenderable = playerEntity.addComponent<PoorCraft::Renderable>(playerMeshData.mesh, playerTexture, std::move(playerSections));
        playerRenderable.setRenderLayer(0);

        playerEntity.addComponent<PoorCraft::AnimationController>();

        auto* playerControllerPtr = playerEntity.getComponent<PoorCraft::PlayerController>();

        if (!playerControllerPtr) {
            PC_FATAL("Failed to initialize player entity components");
            PoorCraft::Renderer::getInstance().shutdown();
            window.shutdown();
            PoorCraft::Window::terminateGLFW();
            PoorCraft::ResourceManager::getInstance().clear();
            PoorCraft::EventBus::getInstance().clear();
            poorcraft::Platform::delete_path(testResourcePath);
            poorcraft::Config::get_instance().save_to_file();
            PC_INFO("PoorCraft engine shutdown complete");
            poorcraft::Logger::get_instance().shutdown();
            return 1;
        }

        auto& playerController = *playerControllerPtr;

#if POORCRAFT_HAS_DEBUG_RENDERER
        bool debugEnabled = false;
#endif

        // Set update callback with ECS-driven player
        gameLoop.setUpdateCallback([&](float deltaTime) {
            auto& input = PoorCraft::Input::getInstance();

            playerTransform.updatePrevious();

            glm::vec3 wishDirection(0.0f);
            if (input.isKeyHeld(GLFW_KEY_W)) {
                wishDirection.z += 1.0f;
            }
            if (input.isKeyHeld(GLFW_KEY_S)) {
                wishDirection.z -= 1.0f;
            }
            if (input.isKeyHeld(GLFW_KEY_A)) {
                wishDirection.x -= 1.0f;
            }
            if (input.isKeyHeld(GLFW_KEY_D)) {
                wishDirection.x += 1.0f;
            }

            PoorCraft::Player& playerInstance = playerController.getPlayer();

            if (glm::length2(wishDirection) > 0.0f) {
                wishDirection = glm::normalize(wishDirection);
            }

            if (playerInstance.isFlying()) {
                float vertical = 0.0f;
                if (input.isKeyHeld(GLFW_KEY_SPACE)) {
                    vertical += 1.0f;
                }
                if (input.isKeyHeld(GLFW_KEY_LEFT_SHIFT)) {
                    vertical -= 1.0f;
                }
                wishDirection.y = vertical;
            }

            bool sprinting = input.isKeyHeld(GLFW_KEY_LEFT_SHIFT) && !playerInstance.isFlying();
            bool jumpRequested = !playerInstance.isFlying() && input.wasKeyJustPressed(GLFW_KEY_SPACE);
            bool flyToggle = input.wasKeyJustPressed(GLFW_KEY_F);
            bool swimToggle = false;

            playerController.handleInput(wishDirection, sprinting, jumpRequested, flyToggle, swimToggle);
            playerController.update(deltaTime);
            playerController.syncTransform(playerTransform);

            animationSystem.update(deltaTime);

            float sensitivity = config.get_float(poorcraft::Config::ControlsConfig::MOUSE_SENSITIVITY_KEY, 1.0f) * 0.1f;
            if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                auto mouseDelta = input.getMouseDelta();
                float yaw = mouseDelta.x * sensitivity * deltaTime;
                float pitch = mouseDelta.y * sensitivity * deltaTime;
                camera.rotate(yaw, pitch);
            }

#if POORCRAFT_HAS_DEBUG_RENDERER
            if (input.wasKeyJustPressed(GLFW_KEY_F3)) {
                debugEnabled = !debugEnabled;
                PoorCraft::DebugRenderer::getInstance().setEnabled(debugEnabled);
            }

            auto& debugRenderer = PoorCraft::DebugRenderer::getInstance();
            if (debugEnabled) {
                debugRenderer.clear();
                debugRenderer.clear();
                debugRenderer.drawAABB(playerInstance.getBounds(), glm::vec3(1.0f, 0.0f, 0.0f));
                const auto velocity = playerInstance.getVelocity();
                const float velocityLengthSq = glm::length2(velocity);
                if (velocityLengthSq > 0.0f) {
                    const float velocityLength = std::sqrt(velocityLengthSq);
                    debugRenderer.drawRay(playerInstance.getPosition(), glm::normalize(velocity), velocityLength, glm::vec3(0.0f, 0.0f, 1.0f));
                }
                const auto target = playerInstance.getTargetBlock();
                if (target.hit) {
                    auto blockAABB = physicsWorld->getBlockAABB(target.blockPos.x, target.blockPos.y, target.blockPos.z);
                    debugRenderer.drawAABB(blockAABB, glm::vec3(0.0f, 1.0f, 0.0f));
                }
            }
#endif

            world->update(playerTransform.getPosition(), renderDistance);
        });

        // Set render callback with renderer API
        gameLoop.setRenderCallback([&]() {
            // Use renderer API instead of raw OpenGL calls
            auto& renderer = PoorCraft::Renderer::getInstance();
            
            // Begin frame
            renderer.beginFrame();
            
            // Set clear color and clear
            renderer.setClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
            renderer.clear();
            
            // Load block shader if not already loaded
            static std::shared_ptr<PoorCraft::Shader> blockShader = nullptr;
            static bool shaderLoaded = false;

            if (!shaderLoaded) {
                try {
                    blockShader = PoorCraft::ResourceManager::getInstance().load<PoorCraft::Shader>("shaders/basic/block");
                    if (!blockShader || !blockShader->isValid()) {
                        PC_WARN("Failed to load block shader, using default shader");
                        blockShader = renderer.getDefaultShader();
                    } else {
                        PC_INFO("Successfully loaded block shader");
                    }
                } catch (const std::exception& e) {
                    PC_ERROR("Error loading block shader: " + std::string(e.what()));
                    blockShader = renderer.getDefaultShader();
                }
                shaderLoaded = true;
            }

            if (blockShader) {
                blockShader->bind();
                world->render(camera, *blockShader);
            }

            static std::shared_ptr<PoorCraft::Shader> entityShader = nullptr;
            static bool entityShaderLoaded = false;

            if (!entityShaderLoaded) {
                try {
                    entityShader = PoorCraft::ResourceManager::getInstance().load<PoorCraft::Shader>("shaders/basic/entity");
                    if (!entityShader || !entityShader->isValid()) {
                        PC_WARN("Failed to load entity shader, using default shader");
                        entityShader = renderer.getDefaultShader();
                    } else {
                        PC_INFO("Successfully loaded entity shader");
                    }
                } catch (const std::exception& e) {
                    PC_ERROR("Error loading entity shader: " + std::string(e.what()));
                    entityShader = renderer.getDefaultShader();
                }
                entityShaderLoaded = true;
            }

            float alpha = 0.0f;
            const float fixedTimestep = gameLoop.getFixedTimestep();
            if (fixedTimestep > 0.0f) {
                alpha = static_cast<float>(gameLoop.getAccumulator() / fixedTimestep);
            }
            alpha = std::clamp(alpha, 0.0f, 1.0f);

            if (entityShader) {
                entityShader->bind();
                entityRenderer.render(camera, *entityShader, alpha);
            }

#if POORCRAFT_HAS_DEBUG_RENDERER
            if (debugEnabled) {
                PoorCraft::DebugRenderer::getInstance().render(camera);
            }
#endif

            renderer.endFrame();
        });

        // Run the game loop
        PC_INFO("=== Starting Game Loop ===");
        PC_INFO("Press ESC or close the window to exit");
        gameLoop.run();

        // Cleanup
        }

        PoorCraft::Renderer::getInstance().shutdown();
        window.shutdown();
        PoorCraft::Window::terminateGLFW();
        PoorCraft::ResourceManager::getInstance().clear();
        PoorCraft::EventBus::getInstance().clear();

        // Clean up test resource file
        poorcraft::Platform::delete_path(testResourcePath);

        // Shutdown systems
        poorcraft::Config::get_instance().save_to_file();
        PC_INFO("PoorCraft engine shutdown complete");
        poorcraft::Logger::get_instance().shutdown();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error during engine initialization: " << e.what() << std::endl;

        // Try to log the error if logger is available
        try {
            PC_FATAL("Fatal error during engine initialization: " + std::string(e.what()));
        } catch (...) {
            // Logger might not be available, ignore
        }

        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error during engine initialization" << std::endl;

        try {
            PC_FATAL("Unknown fatal error during engine initialization");
        } catch (...) {
            // Logger might not be available, ignore
        }

        return 1;
    }
}

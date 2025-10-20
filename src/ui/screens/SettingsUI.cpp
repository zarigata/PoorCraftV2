#include "poorcraft/ui/screens/SettingsUI.h"

#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"
#include "poorcraft/rendering/Camera.h"
#include "poorcraft/rendering/Renderer.h"
#include "poorcraft/rendering/GPUCapabilities.h"
#include "poorcraft/ui/GameState.h"
#include "poorcraft/window/Window.h"

#include <algorithm>

#include <imgui.h>

namespace PoorCraft {

namespace {
constexpr float MIN_VOLUME = 0.0f;
constexpr float MAX_VOLUME = 1.0f;
constexpr float MIN_SENSITIVITY = 0.1f;
constexpr float MAX_SENSITIVITY = 5.0f;
constexpr int MIN_RENDER_DISTANCE = 2;
constexpr int MAX_RENDER_DISTANCE = 32;
constexpr int MIN_FOV = 60;
constexpr int MAX_FOV = 120;
} // namespace

SettingsUI::SettingsUI(Config& config, Window& window, Renderer& renderer, GameStateManager& stateManager)
    : m_Config(config), m_Window(window), m_Renderer(renderer), m_StateManager(stateManager) {
    m_Resolutions = {
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440},
        {3440, 1440},
        {3840, 2160}
    };
    loadCurrentSettings();
}

void SettingsUI::onEnter() {
    UIScreen::onEnter();
    loadCurrentSettings();
}

void SettingsUI::render() {
    if (!isActive()) {
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f));

    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Settings", nullptr, windowFlags)) {
        if (ImGui::BeginTabBar("SettingsTabs")) {
            if (ImGui::BeginTabItem("Graphics")) {
                if (ImGui::BeginCombo("Resolution", (std::to_string(m_TempWidth) + "x" + std::to_string(m_TempHeight)).c_str())) {
                    for (const auto& [width, height] : m_Resolutions) {
                        bool selected = (width == m_TempWidth && height == m_TempHeight);
                        std::string label = std::to_string(width) + "x" + std::to_string(height);
                        if (ImGui::Selectable(label.c_str(), selected)) {
                            m_TempWidth = width;
                            m_TempHeight = height;
                        }
                        if (selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::Checkbox("Fullscreen", &m_TempFullscreen);
                ImGui::Checkbox("VSync", &m_TempVsync);
                ImGui::SliderInt("Field of View", &m_TempFov, MIN_FOV, MAX_FOV);
                ImGui::SliderInt("Render Distance", &m_TempRenderDistance, MIN_RENDER_DISTANCE, MAX_RENDER_DISTANCE);

                ImGui::Separator();
                ImGui::Text("Rendering Backend");

                // Backend selection combo box
                const char* backends[] = {"OpenGL 4.6", "Vulkan", "Vulkan + Ray Tracing"};
                int currentBackend = m_Config.get_int(Config::GraphicsConfig::RENDERING_BACKEND_KEY, 0);
                ImGui::Combo("Backend", &m_TempRenderingBackend, backends, 3);

                // Show warning if backend change requires restart
                if (m_TempRenderingBackend != currentBackend) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Changing backend requires restart");
                }

                // Show RT options only if Vulkan+RT selected
                if (m_TempRenderingBackend == 2) {
                    ImGui::Separator();
                    ImGui::Text("Ray Tracing Settings");
                    
                    // Check if RT is supported
                    bool rtSupported = GPUCapabilities::getInstance().supportsRayTracingPipeline();
                    if (!rtSupported) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Ray tracing not supported on this GPU");
                        ImGui::TextWrapped("Requires NVIDIA RTX 20-series+, AMD RDNA2+, or Intel Arc");
                    } else {
                        ImGui::SliderFloat("Resolution Scale", &m_TempRTResolutionScale, 0.5f, 2.0f, "%.2f");
                        ImGui::SliderInt("Samples Per Pixel", &m_TempRTSamplesPerPixel, 1, 16);
                        ImGui::SliderInt("Max Bounces", &m_TempRTMaxBounces, 1, 4);
                        ImGui::Checkbox("Enable Reflections", &m_TempRTEnableReflections);
                        ImGui::Checkbox("Enable Shadows", &m_TempRTEnableShadows);
                    }
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Audio")) {
                ImGui::SliderFloat("Master Volume", &m_TempMasterVolume, MIN_VOLUME, MAX_VOLUME);
                ImGui::SliderFloat("Music Volume", &m_TempMusicVolume, MIN_VOLUME, MAX_VOLUME);
                ImGui::SliderFloat("Sound Volume", &m_TempSoundVolume, MIN_VOLUME, MAX_VOLUME);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Controls")) {
                ImGui::SliderFloat("Mouse Sensitivity", &m_TempMouseSensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);
                ImGui::Checkbox("Invert Y-Axis", &m_TempInvertY);
                ImGui::TextUnformatted("Keybindings customization coming soon");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Video")) {
                ImGui::TextUnformatted("Advanced graphics options coming soon");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Separator();
        if (ImGui::Button("Apply", ImVec2(120.0f, 0.0f))) {
            applyGraphicsSettings();
            applyAudioSettings();
            applyControlSettings();

            if (!m_Config.save_to_file("config.ini")) {
                PC_WARN("SettingsUI: Failed to save configuration to file");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Back", ImVec2(120.0f, 0.0f))) {
            m_StateManager.popState();
        }
    }
    ImGui::End();
}

void SettingsUI::applyGraphicsSettings() {
    m_TempWidth = std::max(640, m_TempWidth);
    m_TempHeight = std::max(480, m_TempHeight);
    m_TempFov = std::clamp(m_TempFov, MIN_FOV, MAX_FOV);
    m_TempRenderDistance = std::clamp(m_TempRenderDistance, MIN_RENDER_DISTANCE, MAX_RENDER_DISTANCE);

    m_Config.set_int(Config::GraphicsConfig::WIDTH_KEY, m_TempWidth);
    m_Config.set_int(Config::GraphicsConfig::HEIGHT_KEY, m_TempHeight);
    m_Config.set_bool(Config::GraphicsConfig::FULLSCREEN_KEY, m_TempFullscreen);
    m_Config.set_bool(Config::GraphicsConfig::VSYNC_KEY, m_TempVsync);
    m_Config.set_int(Config::GraphicsConfig::FOV_KEY, m_TempFov);
    m_Config.set_int(Config::GameplayConfig::RENDER_DISTANCE_KEY, m_TempRenderDistance);

    // Save backend settings
    m_Config.set_int(Config::GraphicsConfig::RENDERING_BACKEND_KEY, m_TempRenderingBackend);
    m_Config.set_float(Config::GraphicsConfig::RT_RESOLUTION_SCALE_KEY, m_TempRTResolutionScale);
    m_Config.set_int(Config::GraphicsConfig::RT_SAMPLES_PER_PIXEL_KEY, m_TempRTSamplesPerPixel);
    m_Config.set_int(Config::GraphicsConfig::RT_MAX_BOUNCES_KEY, m_TempRTMaxBounces);
    m_Config.set_bool(Config::GraphicsConfig::RT_ENABLE_REFLECTIONS_KEY, m_TempRTEnableReflections);
    m_Config.set_bool(Config::GraphicsConfig::RT_ENABLE_SHADOWS_KEY, m_TempRTEnableShadows);

    m_Window.setSize(static_cast<uint32_t>(m_TempWidth), static_cast<uint32_t>(m_TempHeight));
    m_Window.setFullscreen(m_TempFullscreen);
    m_Window.setVSync(m_TempVsync);

    float aspect = static_cast<float>(m_TempWidth) / static_cast<float>(m_TempHeight);
    if (auto* camera = m_Renderer.getCamera()) {
        camera->updateProjectionMatrix(static_cast<float>(m_TempFov), aspect, 0.1f, 1000.0f);
    }

    PC_INFO("SettingsUI: Graphics settings applied");
}

void SettingsUI::applyAudioSettings() {
    m_TempMasterVolume = std::clamp(m_TempMasterVolume, MIN_VOLUME, MAX_VOLUME);
    m_TempMusicVolume = std::clamp(m_TempMusicVolume, MIN_VOLUME, MAX_VOLUME);
    m_TempSoundVolume = std::clamp(m_TempSoundVolume, MIN_VOLUME, MAX_VOLUME);

    m_Config.set_float(Config::AudioConfig::MASTER_VOLUME_KEY, m_TempMasterVolume);
    m_Config.set_float(Config::AudioConfig::MUSIC_VOLUME_KEY, m_TempMusicVolume);
    m_Config.set_float(Config::AudioConfig::SOUND_VOLUME_KEY, m_TempSoundVolume);

    PC_INFO("SettingsUI: Audio settings applied");
}

void SettingsUI::applyControlSettings() {
    m_TempMouseSensitivity = std::clamp(m_TempMouseSensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);

    m_Config.set_float(Config::ControlsConfig::MOUSE_SENSITIVITY_KEY, m_TempMouseSensitivity);
    m_Config.set_bool(Config::ControlsConfig::INVERT_Y_KEY, m_TempInvertY);

    PC_INFO("SettingsUI: Control settings applied");
}

void SettingsUI::loadCurrentSettings() {
    m_TempWidth = m_Config.get_int(Config::GraphicsConfig::WIDTH_KEY, m_TempWidth);
    m_TempHeight = m_Config.get_int(Config::GraphicsConfig::HEIGHT_KEY, m_TempHeight);
    m_TempFullscreen = m_Config.get_bool(Config::GraphicsConfig::FULLSCREEN_KEY, m_TempFullscreen);
    m_TempVsync = m_Config.get_bool(Config::GraphicsConfig::VSYNC_KEY, m_TempVsync);
    m_TempFov = m_Config.get_int(Config::GraphicsConfig::FOV_KEY, m_TempFov);
    m_TempRenderDistance = m_Config.get_int(Config::GameplayConfig::RENDER_DISTANCE_KEY, m_TempRenderDistance);

    // Load backend settings
    m_TempRenderingBackend = m_Config.get_int(Config::GraphicsConfig::RENDERING_BACKEND_KEY, 0);
    m_TempRTResolutionScale = m_Config.get_float(Config::GraphicsConfig::RT_RESOLUTION_SCALE_KEY, 1.0f);
    m_TempRTSamplesPerPixel = m_Config.get_int(Config::GraphicsConfig::RT_SAMPLES_PER_PIXEL_KEY, 1);
    m_TempRTMaxBounces = m_Config.get_int(Config::GraphicsConfig::RT_MAX_BOUNCES_KEY, 1);
    m_TempRTEnableReflections = m_Config.get_bool(Config::GraphicsConfig::RT_ENABLE_REFLECTIONS_KEY, false);
    m_TempRTEnableShadows = m_Config.get_bool(Config::GraphicsConfig::RT_ENABLE_SHADOWS_KEY, true);

    m_TempMasterVolume = m_Config.get_float(Config::AudioConfig::MASTER_VOLUME_KEY, m_TempMasterVolume);
    m_TempMusicVolume = m_Config.get_float(Config::AudioConfig::MUSIC_VOLUME_KEY, m_TempMusicVolume);
    m_TempSoundVolume = m_Config.get_float(Config::AudioConfig::SOUND_VOLUME_KEY, m_TempSoundVolume);

    m_TempMouseSensitivity = m_Config.get_float(Config::ControlsConfig::MOUSE_SENSITIVITY_KEY, m_TempMouseSensitivity);
    m_TempInvertY = m_Config.get_bool(Config::ControlsConfig::INVERT_Y_KEY, m_TempInvertY);
}

} // namespace PoorCraft

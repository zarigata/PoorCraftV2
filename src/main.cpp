#include "poorcraft/core/Logger.h"
#include "poorcraft/core/Config.h"
#include "poorcraft/platform/Platform.h"

#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
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

        // Test platform utilities
        PC_INFO("=== Platform Information ===");
        PC_INFO("System Info:");
        PC_INFO(poorcraft::Platform::get_system_info());
        PC_INFO("Home Directory: " + poorcraft::Platform::get_home_directory());
        PC_INFO("Temp Directory: " + poorcraft::Platform::get_temp_directory());

        // Test file operations
        std::string test_file = poorcraft::Platform::create_temp_file_path("poorcraft_test", ".txt");
        if (!test_file.empty()) {
            PC_DEBUG("Created temp file path: " + test_file);

            // Write test content
            std::string test_content = "PoorCraft engine test file\nGenerated: " +
                                     std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            poorcraft::Platform::FileOperationResult result =
                poorcraft::Platform::write_file_text(test_file, test_content);

            if (result == poorcraft::Platform::FileOperationResult::Success) {
                PC_DEBUG("Successfully wrote test file");

                // Read it back
                std::string read_content;
                result = poorcraft::Platform::read_file_text(test_file, read_content);

                if (result == poorcraft::Platform::FileOperationResult::Success) {
                    PC_DEBUG("Successfully read test file: " + read_content.substr(0, 50) + "...");
                } else {
                    PC_WARN("Failed to read test file");
                }
            } else {
                PC_WARN("Failed to write test file");
            }

            // Clean up
            poorcraft::Platform::delete_path(test_file);
        }

        // Test directory operations
        std::string test_dir = poorcraft::Platform::join_path(
            poorcraft::Platform::get_temp_directory(), "poorcraft_test_dir");

        poorcraft::Platform::FileOperationResult result =
            poorcraft::Platform::create_directory(test_dir, true);

        if (result == poorcraft::Platform::FileOperationResult::Success ||
            result == poorcraft::Platform::FileOperationResult::AlreadyExists) {
            PC_DEBUG("Successfully created test directory: " + test_dir);

            // List directory contents
            std::vector<std::string> entries;
            result = poorcraft::Platform::list_directory(test_dir, entries);

            if (result == poorcraft::Platform::FileOperationResult::Success) {
                PC_DEBUG("Directory contents (" + std::to_string(entries.size()) + " entries)");
            }

            // Clean up
            poorcraft::Platform::delete_path(test_dir, true);
        } else {
            PC_WARN("Failed to create test directory");
        }

        PC_INFO("=== Core Systems Test Complete ===");
        PC_INFO("All core systems (Logger, Config, Platform) initialized successfully!");
        PC_INFO("PoorCraft engine foundation is ready for development.");

        // In a real engine, this would be the main game loop
        // For now, we'll just exit successfully

        // Shutdown systems
        poorcraft::Config::get_instance().save_to_file();
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

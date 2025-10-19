#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>

namespace poorcraft {

/**
 * @brief Configuration change callback function type
 */
using ConfigChangeCallback = std::function<void(const std::string& key, const std::string& value)>;

/**
 * @brief Singleton configuration manager for engine settings
 *
 * The Config class provides a centralized configuration system that supports
 * loading from and saving to INI-style configuration files. It offers type-safe
 * access methods for common data types with default value support.
 */
class Config {
public:
    /**
     * @brief Get the singleton instance of the configuration manager
     * @return Reference to the config instance
     */
    static Config& get_instance();

    /**
     * @brief Load configuration from a file
     * @param file_path Path to the configuration file
     * @return True if loaded successfully, false otherwise
     */
    bool load_from_file(const std::string& file_path);

    /**
     * @brief Save current configuration to a file
     * @param file_path Path to save the configuration file
     * @return True if saved successfully, false otherwise
     */
    bool save_to_file(const std::string& file_path = "");

    /**
     * @brief Check if a configuration key exists
     * @param key Configuration key to check
     * @return True if the key exists, false otherwise
     */
    bool has(const std::string& key) const;

    /**
     * @brief Get a string configuration value
     * @param key Configuration key
     * @param default_value Default value if key doesn't exist
     * @return Configuration value or default
     */
    std::string get_string(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Get an integer configuration value
     * @param key Configuration key
     * @param default_value Default value if key doesn't exist or conversion fails
     * @return Configuration value or default
     */
    int get_int(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get a float configuration value
     * @param key Configuration key
     * @param default_value Default value if key doesn't exist or conversion fails
     * @return Configuration value or default
     */
    float get_float(const std::string& key, float default_value = 0.0f) const;

    /**
     * @brief Get a boolean configuration value
     * @param key Configuration key
     * @param default_value Default value if key doesn't exist or conversion fails
     * @return Configuration value or default
     */
    bool get_bool(const std::string& key, bool default_value = false) const;

    /**
     * @brief Set a string configuration value
     * @param key Configuration key
     * @param value Value to set
     * @param trigger_callback Whether to trigger change callbacks
     */
    void set_string(const std::string& key, const std::string& value, bool trigger_callback = true);

    /**
     * @brief Set an integer configuration value
     * @param key Configuration key
     * @param value Value to set
     * @param trigger_callback Whether to trigger change callbacks
     */
    void set_int(const std::string& key, int value, bool trigger_callback = true);

    /**
     * @brief Set a float configuration value
     * @param key Configuration key
     * @param value Value to set
     * @param trigger_callback Whether to trigger change callbacks
     */
    void set_float(const std::string& key, float value, bool trigger_callback = true);

    /**
     * @brief Set a boolean configuration value
     * @param key Configuration key
     * @param value Value to set
     * @param trigger_callback Whether to trigger change callbacks
     */
    void set_bool(const std::string& key, bool value, bool trigger_callback = true);

    /**
     * @brief Generic get method for any type with automatic conversion
     * @tparam T Type to convert to
     * @param key Configuration key
     * @param default_value Default value if key doesn't exist
     * @return Configuration value or default
     */
    template<typename T>
    T get(const std::string& key, const T& default_value) const;

    /**
     * @brief Generic set method for any type
     * @tparam T Type of value to set
     * @param key Configuration key
     * @param value Value to set
     * @param trigger_callback Whether to trigger change callbacks
     */
    template<typename T>
    void set(const std::string& key, const T& value, bool trigger_callback = true);

    /**
     * @brief Remove a configuration key
     * @param key Configuration key to remove
     * @param trigger_callback Whether to trigger change callbacks
     * @return True if key was removed, false if it didn't exist
     */
    bool remove(const std::string& key, bool trigger_callback = true);

    /**
     * @brief Clear all configuration values
     * @param trigger_callback Whether to trigger change callbacks for each removal
     */
    void clear(bool trigger_callback = false);

    /**
     * @brief Get the number of configuration entries
     * @return Number of configuration keys
     */
    size_t size() const;

    /**
     * @brief Check if configuration is empty
     * @return True if no configuration keys exist
     */
    bool empty() const;

    /**
     * @brief Register a callback for configuration changes
     * @param key Key to monitor (empty string for all keys)
     * @param callback Callback function to invoke on changes
     * @return Unique callback ID for removal
     */
    size_t register_change_callback(const std::string& key, ConfigChangeCallback callback);

    /**
     * @brief Unregister a configuration change callback
     * @param callback_id ID returned by register_change_callback
     * @return True if callback was removed, false if not found
     */
    bool unregister_change_callback(size_t callback_id);

    /**
     * @brief Get all configuration keys
     * @return Vector of all configuration keys
     */
    std::vector<std::string> get_keys() const;

    /**
     * @brief Get all configuration keys in a section
     * @param section Configuration section name
     * @return Vector of keys in the specified section
     */
    std::vector<std::string> get_keys_in_section(const std::string& section) const;

    /**
     * @brief Set the configuration file path for load/save operations
     * @param file_path Path to the configuration file
     */
    void set_config_file_path(const std::string& file_path);

    /**
     * @brief Get the current configuration file path
     * @return Path to the configuration file
     */
    std::string get_config_file_path() const;

    // Predefined configuration sections for organization

    /**
     * @brief Graphics configuration section
     */
    struct GraphicsConfig {
        static constexpr const char* SECTION = "Graphics";
        static constexpr const char* WIDTH_KEY = "Graphics.width";
        static constexpr const char* HEIGHT_KEY = "Graphics.height";
        static constexpr const char* FULLSCREEN_KEY = "Graphics.fullscreen";
        static constexpr const char* VSYNC_KEY = "Graphics.vsync";
        static constexpr const char* FOV_KEY = "Graphics.fov";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* WIDTH = "width";
        static constexpr const char* HEIGHT = "height";
        static constexpr const char* FULLSCREEN = "fullscreen";
        static constexpr const char* VSYNC = "vsync";
        static constexpr const char* FOV = "fov";
    };

    /**
     * @brief Audio configuration section
     */
    struct AudioConfig {
        static constexpr const char* SECTION = "Audio";
        static constexpr const char* MASTER_VOLUME_KEY = "Audio.master_volume";
        static constexpr const char* MUSIC_VOLUME_KEY = "Audio.music_volume";
        static constexpr const char* SOUND_VOLUME_KEY = "Audio.sound_volume";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* MASTER_VOLUME = "master_volume";
        static constexpr const char* MUSIC_VOLUME = "music_volume";
        static constexpr const char* SOUND_VOLUME = "sound_volume";
    };

    /**
     * @brief Controls configuration section
     */
    struct ControlsConfig {
        static constexpr const char* SECTION = "Controls";
        static constexpr const char* MOUSE_SENSITIVITY_KEY = "Controls.mouse_sensitivity";
        static constexpr const char* INVERT_Y_KEY = "Controls.invert_y";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* MOUSE_SENSITIVITY = "mouse_sensitivity";
        static constexpr const char* INVERT_Y = "invert_y";
    };

    /**
     * @brief Gameplay configuration section
     */
    struct GameplayConfig {
        static constexpr const char* SECTION = "Gameplay";
        static constexpr const char* RENDER_DISTANCE_KEY = "Gameplay.render_distance";
        static constexpr const char* DIFFICULTY_KEY = "Gameplay.difficulty";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* RENDER_DISTANCE = "render_distance";
        static constexpr const char* DIFFICULTY = "difficulty";
    };

    /**
     * @brief Network configuration section
     */
    struct NetworkConfig {
        static constexpr const char* SECTION = "Network";
        static constexpr const char* DEFAULT_PORT_KEY = "Network.default_port";
        static constexpr const char* TIMEOUT_KEY = "Network.timeout";
        static constexpr const char* MAX_PLAYERS_KEY = "Network.max_players";
        static constexpr const char* SERVER_NAME_KEY = "Network.server_name";
        static constexpr const char* SERVER_DESCRIPTION_KEY = "Network.server_description";
        static constexpr const char* MAX_CONNECTION_ATTEMPTS_KEY = "Network.max_connection_attempts";
        static constexpr const char* HANDSHAKE_TIMEOUT_KEY = "Network.handshake_timeout";
        static constexpr const char* NETWORK_TICK_RATE_KEY = "Network.network_tick_rate";
        static constexpr const char* SNAPSHOT_RATE_KEY = "Network.snapshot_rate";
        static constexpr const char* MAX_PACKET_SIZE_KEY = "Network.max_packet_size";
        static constexpr const char* CHUNK_SEND_RATE_KEY = "Network.chunk_send_rate";
        static constexpr const char* CHUNK_COMPRESSION_KEY = "Network.chunk_compression";
        static constexpr const char* ENABLE_PREDICTION_KEY = "Network.enable_prediction";
        static constexpr const char* PREDICTION_ERROR_THRESHOLD_KEY = "Network.prediction_error_threshold";
        static constexpr const char* INTERPOLATION_DELAY_KEY = "Network.interpolation_delay";
        static constexpr const char* MAX_INCOMING_BANDWIDTH_KEY = "Network.max_incoming_bandwidth";
        static constexpr const char* MAX_OUTGOING_BANDWIDTH_KEY = "Network.max_outgoing_bandwidth";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* DEFAULT_PORT = "default_port";
        static constexpr const char* TIMEOUT = "timeout";
    };

    /**
     * @brief Engine configuration section
     */
    struct EngineConfig {
        static constexpr const char* SECTION = "Engine";
        static constexpr const char* LOG_LEVEL_KEY = "Engine.log_level";
        static constexpr const char* MAX_FPS_KEY = "Engine.max_fps";

        // Legacy constants for backward compatibility (deprecated)
        static constexpr const char* LOG_LEVEL = "log_level";
        static constexpr const char* MAX_FPS = "max_fps";
    };

    struct UIConfig {
        static constexpr const char* SECTION = "UI";
        static constexpr const char* UI_SCALE_KEY = "UI.ui_scale";
        static constexpr const char* UI_THEME_KEY = "UI.ui_theme";
        static constexpr const char* SHOW_FPS_KEY = "UI.show_fps";
        static constexpr const char* SHOW_COORDINATES_KEY = "UI.show_coordinates";
        static constexpr const char* SHOW_DEBUG_INFO_KEY = "UI.show_debug_info";
        static constexpr const char* CHAT_MAX_MESSAGES_KEY = "UI.chat_max_messages";
        static constexpr const char* CHAT_FADE_TIME_KEY = "UI.chat_fade_time";
        static constexpr const char* HUD_OPACITY_KEY = "UI.hud_opacity";
    };

    struct PhysicsConfig {
        static constexpr const char* SECTION = "Physics";
        static constexpr const char* WALK_SPEED_KEY = "Physics.walk_speed";
        static constexpr const char* SPRINT_SPEED_KEY = "Physics.sprint_speed";
        static constexpr const char* FLY_SPEED_KEY = "Physics.fly_speed";
        static constexpr const char* SWIM_SPEED_KEY = "Physics.swim_speed";
        static constexpr const char* GRAVITY_KEY = "Physics.gravity";
        static constexpr const char* JUMP_FORCE_KEY = "Physics.jump_force";
        static constexpr const char* GROUND_FRICTION_KEY = "Physics.ground_friction";
        static constexpr const char* AIR_FRICTION_KEY = "Physics.air_friction";
        static constexpr const char* WATER_FRICTION_KEY = "Physics.water_friction";
        static constexpr const char* ACCELERATION_KEY = "Physics.acceleration";
        static constexpr const char* PLAYER_WIDTH_KEY = "Physics.player_width";
        static constexpr const char* PLAYER_HEIGHT_KEY = "Physics.player_height";
        static constexpr const char* PLAYER_EYE_HEIGHT_KEY = "Physics.player_eye_height";
        static constexpr const char* STEP_HEIGHT_KEY = "Physics.step_height";
        static constexpr const char* REACH_DISTANCE_KEY = "Physics.reach_distance";

        static constexpr const char* WALK_SPEED = "walk_speed";
        static constexpr const char* SPRINT_SPEED = "sprint_speed";
        static constexpr const char* FLY_SPEED = "fly_speed";
        static constexpr const char* SWIM_SPEED = "swim_speed";
        static constexpr const char* GRAVITY = "gravity";
        static constexpr const char* JUMP_FORCE = "jump_force";
        static constexpr const char* GROUND_FRICTION = "ground_friction";
        static constexpr const char* AIR_FRICTION = "air_friction";
        static constexpr const char* WATER_FRICTION = "water_friction";
        static constexpr const char* ACCELERATION = "acceleration";
        static constexpr const char* PLAYER_WIDTH = "player_width";
        static constexpr const char* PLAYER_HEIGHT = "player_height";
        static constexpr const char* PLAYER_EYE_HEIGHT = "player_eye_height";
        static constexpr const char* STEP_HEIGHT = "step_height";
        static constexpr const char* REACH_DISTANCE = "reach_distance";
    };

private:
    Config() = default;
    ~Config() = default;

    // Disable copy and assignment
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    /**
     * @brief Parse a configuration file line
     * @param line Line to parse
     * @param current_section Current section name (output parameter)
     * @return True if line was parsed successfully
     */
    bool parse_line(const std::string& line, std::string& current_section);

    /**
     * @brief Convert string value to specified type
     */
    template<typename T>
    T convert_string(const std::string& value, const T& default_value) const;

    /**
     * @brief Convert value to string for storage
     */
    template<typename T>
    std::string convert_to_string(const T& value) const;

    /**
     * @brief Trigger change callbacks for a key
     */
    void trigger_callbacks(const std::string& key, const std::string& value);

private:
    std::unordered_map<std::string, std::string> config_data_;
    std::string config_file_path_;
    mutable std::mutex mutex_;

    // Change callback management
    std::unordered_map<size_t, std::pair<std::string, ConfigChangeCallback>> callbacks_;
    size_t next_callback_id_ = 1;
};

// Template implementations

template<typename T>
T Config::get(const std::string& key, const T& default_value) const {
    if (!has(key)) {
        return default_value;
    }

    return convert_string(get_string(key), default_value);
}

template<typename T>
void Config::set(const std::string& key, const T& value, bool trigger_callback) {
    std::string string_value = convert_to_string(value);
    set_string(key, string_value, trigger_callback);
}

template<typename T>
T Config::convert_string(const std::string& value, const T& default_value) const {
    // This would need specialization for different types
    // For now, return default value
    return default_value;
}

template<typename T>
std::string Config::convert_to_string(const T& value) const {
    // This would need specialization for different types
    // For now, return empty string
    return "";
}

// Specialization for int
template<>
int Config::convert_string<int>(const std::string& value, const int& default_value) const;

template<>
std::string Config::convert_to_string<int>(const int& value) const;

// Specialization for float
template<>
float Config::convert_string<float>(const std::string& value, const float& default_value) const;

template<>
std::string Config::convert_to_string<float>(const float& value) const;

// Specialization for bool
template<>
bool Config::convert_string<bool>(const std::string& value, const bool& default_value) const;

template<>
std::string Config::convert_to_string<bool>(const bool& value) const;

} // namespace poorcraft

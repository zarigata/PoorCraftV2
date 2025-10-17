#include "poorcraft/core/Config.h"
#include "poorcraft/core/Logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace poorcraft {

Config& Config::get_instance() {
    static Config instance;
    return instance;
}

bool Config::load_from_file(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ifstream file(file_path);
    if (!file.is_open()) {
        PC_WARN("Failed to open configuration file: " + file_path);
        return false;
    }

    config_data_.clear();
    std::string current_section;
    std::string line;
    size_t line_number = 0;

    while (std::getline(file, line)) {
        line_number++;

        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (!parse_line(line, current_section)) {
            PC_WARN("Failed to parse configuration line " + std::to_string(line_number) + ": " + line);
        }
    }

    file.close();
    config_file_path_ = file_path;

    PC_INFO("Loaded configuration from " + file_path + " (" + std::to_string(config_data_.size()) + " entries)");
    return true;
}

bool Config::save_to_file(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string path = file_path.empty() ? config_file_path_ : file_path;
    if (path.empty()) {
        PC_ERROR("No configuration file path specified for saving");
        return false;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        PC_ERROR("Failed to open configuration file for writing: " + path);
        return false;
    }

    // Group keys by section for better organization
    std::unordered_map<std::string, std::vector<std::string>> section_keys;

    for (const auto& pair : config_data_) {
        size_t dot_pos = pair.first.find('.');
        std::string section = (dot_pos != std::string::npos) ? pair.first.substr(0, dot_pos) : "General";
        section_keys[section].push_back(pair.first);
    }

    // Write sections in order
    std::vector<std::string> sections;
    for (const auto& pair : section_keys) {
        sections.push_back(pair.first);
    }
    std::sort(sections.begin(), sections.end());

    for (const std::string& section : sections) {
        file << "[" << section << "]" << std::endl;

        std::vector<std::string> keys = section_keys[section];
        std::sort(keys.begin(), keys.end());

        for (const std::string& key : keys) {
            file << key.substr(key.find('.') + 1) << "=" << config_data_[key] << std::endl;
        }
        file << std::endl;
    }

    file.close();

    if (!file_path.empty()) {
        config_file_path_ = file_path;
    }

    PC_INFO("Saved configuration to " + path + " (" + std::to_string(config_data_.size()) + " entries)");
    return true;
}

bool Config::has(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_data_.find(key) != config_data_.end();
}

std::string Config::get_string(const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_data_.find(key);
    return (it != config_data_.end()) ? it->second : default_value;
}

int Config::get_int(const std::string& key, int default_value) const {
    return get<int>(key, default_value);
}

float Config::get_float(const std::string& key, float default_value) const {
    return get<float>(key, default_value);
}

bool Config::get_bool(const std::string& key, bool default_value) const {
    return get<bool>(key, default_value);
}

void Config::set_string(const std::string& key, const std::string& value, bool trigger_callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string old_value;
    auto it = config_data_.find(key);
    if (it != config_data_.end()) {
        old_value = it->second;
    }

    config_data_[key] = value;

    if (trigger_callback) {
        trigger_callbacks(key, value);
    }
}

void Config::set_int(const std::string& key, int value, bool trigger_callback) {
    set<int>(key, value, trigger_callback);
}

void Config::set_float(const std::string& key, float value, bool trigger_callback) {
    set<float>(key, value, trigger_callback);
}

void Config::set_bool(const std::string& key, bool value, bool trigger_callback) {
    set<bool>(key, value, trigger_callback);
}

bool Config::remove(const std::string& key, bool trigger_callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = config_data_.find(key);
    if (it == config_data_.end()) {
        return false;
    }

    std::string old_value = it->second;
    config_data_.erase(it);

    if (trigger_callback) {
        trigger_callbacks(key, "");
    }

    return true;
}

void Config::clear(bool trigger_callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (trigger_callback) {
        for (const auto& pair : config_data_) {
            trigger_callbacks(pair.first, "");
        }
    }

    config_data_.clear();
}

size_t Config::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_data_.size();
}

bool Config::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_data_.empty();
}

size_t Config::register_change_callback(const std::string& key, ConfigChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t id = next_callback_id_++;
    callbacks_[id] = std::make_pair(key, callback);
    return id;
}

bool Config::unregister_change_callback(size_t callback_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.erase(callback_id) > 0;
}

std::vector<std::string> Config::get_keys() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> keys;
    keys.reserve(config_data_.size());

    for (const auto& pair : config_data_) {
        keys.push_back(pair.first);
    }

    return keys;
}

std::vector<std::string> Config::get_keys_in_section(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> keys;

    for (const auto& pair : config_data_) {
        if (pair.first.substr(0, pair.first.find('.')) == section) {
            keys.push_back(pair.first);
        }
    }

    return keys;
}

void Config::set_config_file_path(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_file_path_ = file_path;
}

std::string Config::get_config_file_path() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_file_path_;
}

bool Config::parse_line(const std::string& line, std::string& current_section) {
    if (line.empty()) {
        return true;
    }

    // Check if this is a section header
    if (line[0] == '[' && line.back() == ']') {
        current_section = line.substr(1, line.length() - 2);
        return true;
    }

    // Parse key=value pair
    size_t equals_pos = line.find('=');
    if (equals_pos == std::string::npos) {
        return false;
    }

    std::string key = line.substr(0, equals_pos);
    std::string value = line.substr(equals_pos + 1);

    // Trim whitespace
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    if (key.empty()) {
        return false;
    }

    // Construct full key with section
    std::string full_key = current_section.empty() ? key : current_section + "." + key;
    config_data_[full_key] = value;

    return true;
}

void Config::trigger_callbacks(const std::string& key, const std::string& value) {
    for (const auto& pair : callbacks_) {
        if (pair.second.first.empty() || pair.second.first == key) {
            try {
                pair.second.second(key, value);
            } catch (const std::exception& e) {
                PC_ERROR("Exception in configuration change callback: " + std::string(e.what()));
            }
        }
    }
}

// Template specializations

template<>
int Config::convert_string<int>(const std::string& value, const int& default_value) const {
    if (value.empty()) {
        return default_value;
    }

    char* end_ptr;
    int result = std::strtol(value.c_str(), &end_ptr, 10);
    return (*end_ptr == '\0') ? result : default_value;
}

template<>
std::string Config::convert_to_string<int>(const int& value) const {
    return std::to_string(value);
}

template<>
float Config::convert_string<float>(const std::string& value, const float& default_value) const {
    if (value.empty()) {
        return default_value;
    }

    char* end_ptr;
    float result = std::strtof(value.c_str(), &end_ptr);
    return (*end_ptr == '\0') ? result : default_value;
}

template<>
std::string Config::convert_to_string<float>(const float& value) const {
    // Use enough precision for float but avoid unnecessary decimals
    std::ostringstream oss;
    oss << value;
    std::string str = oss.str();

    // Remove trailing zeros
    str.erase(str.find_last_not_of('0') + 1);
    if (str.back() == '.') {
        str.pop_back();
    }

    return str;
}

template<>
bool Config::convert_string<bool>(const std::string& value, const bool& default_value) const {
    if (value.empty()) {
        return default_value;
    }

    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

    if (lower_value == "true" || lower_value == "1" || lower_value == "yes" || lower_value == "on") {
        return true;
    }
    if (lower_value == "false" || lower_value == "0" || lower_value == "no" || lower_value == "off") {
        return false;
    }

    return default_value;
}

template<>
std::string Config::convert_to_string<bool>(const bool& value) const {
    return value ? "true" : "false";
}

} // namespace poorcraft

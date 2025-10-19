#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace PoorCraft {

// Engine API version - increment on breaking changes
constexpr uint32_t ENGINE_API_VERSION = 1;

/**
 * @brief Mod metadata structure
 */
struct ModMetadata {
    std::string id;              // Unique mod identifier
    std::string name;            // Display name
    std::string version;         // Semantic version (X.Y.Z)
    std::string author;          // Author name
    std::string description;     // Description
    uint32_t apiVersion;         // Engine API version
    std::vector<std::string> dependencies;  // Other mod IDs
    int loadPriority;            // Lower loads first
    bool isNative;               // true for .dll/.so/.dylib, false for .lua
    std::string libraryPath;     // Path to .dll/.so/.dylib or .lua file
    bool enabled;                // From config

    ModMetadata()
        : apiVersion(0)
        , loadPriority(100)
        , isNative(false)
        , enabled(true)
    {}
};

/**
 * @brief Mod manifest parser
 */
class ModManifest {
public:
    /**
     * @brief Parse mod.json manifest file
     * @param manifestPath Path to mod.json
     * @return Parsed metadata
     * @throws std::runtime_error on parse failure
     */
    static ModMetadata parseManifest(const std::string& manifestPath);

    /**
     * @brief Validate mod metadata
     * @param metadata Metadata to validate
     * @return true if valid
     */
    static bool validateMetadata(const ModMetadata& metadata);

private:
    static std::string extractJsonString(const std::string& json, const std::string& key);
    static int extractJsonInt(const std::string& json, const std::string& key, int defaultValue = 0);
    static std::vector<std::string> extractJsonArray(const std::string& json, const std::string& key);
};

} // namespace PoorCraft

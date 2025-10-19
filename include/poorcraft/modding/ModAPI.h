#pragma once

#include <cstdint>

// C API for native plugins - stable ABI across compilers
#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct ModAPI ModAPI;
typedef struct ModInfo ModInfo;

// Event callback signature
typedef void (*EventCallback)(const void* eventData, void* userData);

/**
 * @brief Mod metadata structure
 */
struct ModInfo {
    char name[64];
    char version[16];
    char author[64];
    char description[256];
    uint32_t apiVersion;
};

/**
 * @brief Engine API exposed to mods
 * 
 * All functions return bool for success/failure or appropriate values.
 * Strings are null-terminated C strings.
 */
struct ModAPI {
    // Block API
    uint16_t (*registerBlock)(const char* name, bool isSolid, bool isOpaque, bool isTransparent, 
                              const char* textureName, float hardness);
    uint16_t (*getBlockID)(const char* name);
    bool (*getBlockName)(uint16_t id, char* outBuffer, size_t bufferSize);
    bool (*setBlockAt)(int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t blockId);
    uint16_t (*getBlockAt)(int32_t worldX, int32_t worldY, int32_t worldZ);

    // Entity API
    uint32_t (*spawnEntity)(const char* name, float x, float y, float z);
    bool (*destroyEntity)(uint32_t entityId);
    bool (*getEntityPosition)(uint32_t entityId, float* outX, float* outY, float* outZ);
    bool (*setEntityPosition)(uint32_t entityId, float x, float y, float z);

    // Event API
    uint32_t (*subscribeEvent)(uint32_t eventType, EventCallback callback, void* userData);
    void (*unsubscribeEvent)(uint32_t subscriptionId);
    void (*publishEvent)(uint32_t eventType, const void* eventData);

    // World API
    bool (*getChunkLoaded)(int32_t chunkX, int32_t chunkZ);
    int64_t (*getWorldSeed)();

    // Logging API
    void (*logInfo)(const char* message);
    void (*logWarn)(const char* message);
    void (*logError)(const char* message);

    // Config API
    int32_t (*getConfigInt)(const char* key, int32_t defaultValue);
    float (*getConfigFloat)(const char* key, float defaultValue);
    bool (*getConfigString)(const char* key, const char* defaultValue, char* outBuffer, size_t bufferSize);
    void (*setConfigInt)(const char* key, int32_t value);
    void (*setConfigFloat)(const char* key, float value);
};

/**
 * @brief Plugin entry points (must be implemented by native plugins)
 */

// Get mod metadata (called first for validation)
ModInfo* GetModInfo();

// Initialize mod with engine API (return false on failure)
bool InitializeMod(const ModAPI* api);

// Update mod each server tick (optional - can be no-op)
void UpdateMod(float deltaTime);

// Shutdown mod and cleanup resources
void ShutdownMod();

#ifdef __cplusplus
}
#endif

// C++ helper to create ModAPI struct
#ifdef __cplusplus
namespace PoorCraft {
    ModAPI createModAPI();
}
#endif

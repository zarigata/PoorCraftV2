#pragma once

#include <cstdint>

// C API for native plugins - stable ABI across compilers
#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct ModAPI ModAPI;
typedef struct ModInfo ModInfo;

// Event type constants (matching EventType enum in Event.h)
#define POORCRAFT_EVENT_NONE 0
#define POORCRAFT_EVENT_WINDOW_CLOSE 1
#define POORCRAFT_EVENT_WINDOW_RESIZE 2
#define POORCRAFT_EVENT_WINDOW_FOCUS 3
#define POORCRAFT_EVENT_WINDOW_MINIMIZE 4
#define POORCRAFT_EVENT_WINDOW_MOVE 5
#define POORCRAFT_EVENT_KEY_PRESS 6
#define POORCRAFT_EVENT_KEY_RELEASE 7
#define POORCRAFT_EVENT_MOUSE_MOVE 8
#define POORCRAFT_EVENT_MOUSE_BUTTON_PRESS 9
#define POORCRAFT_EVENT_MOUSE_BUTTON_RELEASE 10
#define POORCRAFT_EVENT_MOUSE_SCROLL 11
#define POORCRAFT_EVENT_GAMEPAD_BUTTON 12
#define POORCRAFT_EVENT_GAMEPAD_AXIS 13
#define POORCRAFT_EVENT_PLAYER_JOINED 14
#define POORCRAFT_EVENT_PLAYER_LEFT 15
#define POORCRAFT_EVENT_CONNECTION_ESTABLISHED 16
#define POORCRAFT_EVENT_CONNECTION_LOST 17
#define POORCRAFT_EVENT_CHUNK_RECEIVED 18
#define POORCRAFT_EVENT_SERVER_STARTED 19
#define POORCRAFT_EVENT_SERVER_STOPPED 20
#define POORCRAFT_EVENT_MOD_LOADED 21
#define POORCRAFT_EVENT_MOD_UNLOADED 22
#define POORCRAFT_EVENT_MOD_RELOADED 23
#define POORCRAFT_EVENT_BLOCK_PLACED 24
#define POORCRAFT_EVENT_BLOCK_BROKEN 25
#define POORCRAFT_EVENT_ENTITY_SPAWNED 26
#define POORCRAFT_EVENT_ENTITY_DESTROYED 27
#define POORCRAFT_EVENT_PLAYER_INTERACT 28
#define POORCRAFT_EVENT_CHUNK_GENERATED 29

// Event callback signature
typedef void (*EventCallback)(const void* eventData, void* userData);

/**
 * @brief C ABI event payload structures
 * 
 * These structs provide stable C-compatible payloads for events.
 * Pointers are valid only for the duration of the callback.
 */

typedef struct PlayerJoinedEventData {
    uint32_t playerId;
    char playerName[64];
    float positionX;
    float positionY;
    float positionZ;
} PlayerJoinedEventData;

typedef struct PlayerLeftEventData {
    uint32_t playerId;
    char playerName[64];
    char reason[128];
} PlayerLeftEventData;

typedef struct BlockPlacedEventData {
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t blockId;
    uint32_t playerId;
    uint16_t previousBlockId;
} BlockPlacedEventData;

typedef struct BlockBrokenEventData {
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t blockId;
    uint32_t playerId;
} BlockBrokenEventData;

typedef struct EntitySpawnedEventData {
    uint32_t entityId;
    char entityName[64];
    float positionX;
    float positionY;
    float positionZ;
    uint32_t spawnedBy;
} EntitySpawnedEventData;

typedef struct EntityDestroyedEventData {
    uint32_t entityId;
    char reason[128];
} EntityDestroyedEventData;

typedef struct PlayerInteractEventData {
    uint32_t playerId;
    int32_t targetX;
    int32_t targetY;
    int32_t targetZ;
    uint16_t targetBlockId;
    uint8_t interactionType; // 0 = LEFT_CLICK, 1 = RIGHT_CLICK
} PlayerInteractEventData;

typedef struct ChunkGeneratedEventData {
    int32_t chunkX;
    int32_t chunkZ;
} ChunkGeneratedEventData;

/**
 * @brief Generic mod event wrapper
 */
typedef struct ModEvent {
    uint32_t eventType;
    const void* data;
} ModEvent;

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
#include <vector>
namespace PoorCraft {
    // Forward declarations
    class EntityManager;
    class World;
    class ChunkManager;
    
    ModAPI createModAPI(EntityManager* entityManager, World* world, ChunkManager* chunkManager);
    
    // Internal: Set current mod context for subscription tracking
    void ModAPI_SetCurrentModContext(std::vector<uint32_t>* subscriptions);
}
#endif

#include "poorcraft/modding/ModAPI.h"
#include <cstring>

// Static mod info
static ModInfo s_ModInfo = {
    "Example Native Mod",
    "1.0.0",
    "PoorCraft Team",
    "Example native plugin demonstrating block registration and event hooks",
    1  // API version
};

// Store API pointer
static const ModAPI* s_API = nullptr;

// Store subscription IDs for cleanup
static uint32_t s_BlockPlacedSubscription = 0;

// Event callback for block placed
void OnBlockPlaced(const void* eventData, void* userData) {
    if (s_API && eventData) {
        const BlockPlacedEventData* data = static_cast<const BlockPlacedEventData*>(eventData);
        char msg[256];
        snprintf(msg, sizeof(msg), "Example Native Mod: Block %u placed at (%d, %d, %d) by player %u", 
                 data->blockId, data->x, data->y, data->z, data->playerId);
        s_API->logInfo(msg);
    }
}

// Plugin entry points
extern "C" {

ModInfo* GetModInfo() {
    return &s_ModInfo;
}

bool InitializeMod(const ModAPI* api) {
    s_API = api;
    
    if (!s_API) {
        return false;
    }
    
    s_API->logInfo("Example Native Mod initializing...");
    
    // Register a custom block
    uint16_t blockId = s_API->registerBlock(
        "example_block",      // name
        true,                 // isSolid
        true,                 // isOpaque
        false,                // isTransparent
        "stone",              // textureName (reuse stone texture)
        2.0f                  // hardness
    );
    
    if (blockId > 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Registered custom block 'example_block' with ID: %u", blockId);
        s_API->logInfo(msg);
    } else {
        s_API->logError("Failed to register custom block");
    }
    
    // Subscribe to block placed events
    s_BlockPlacedSubscription = s_API->subscribeEvent(
        POORCRAFT_EVENT_BLOCK_PLACED,
        OnBlockPlaced,
        nullptr
    );
    
    s_API->logInfo("Example Native Mod initialized successfully!");
    return true;
}

void UpdateMod(float deltaTime) {
    // Per-tick logic (optional)
    // Keep this lightweight - called at 60Hz
}

void ShutdownMod() {
    if (s_API) {
        s_API->logInfo("Example Native Mod shutting down...");
        
        // Unsubscribe from events
        if (s_BlockPlacedSubscription > 0) {
            s_API->unsubscribeEvent(s_BlockPlacedSubscription);
        }
        
        s_API->logInfo("Example Native Mod shut down");
    }
}

} // extern "C"

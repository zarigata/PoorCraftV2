# PoorCraft Modding Guide

## Overview
PoorCraft supports server-side mods through two extensibility layers:
- **Native Plugins**: C/C++ shared libraries (.dll/.so/.dylib) for performance-critical mods
- **Lua Scripts**: Lua 5.4 scripts for gameplay logic and event handling

All mods run on the authoritative server only. Changes are automatically synced to clients via the existing network protocol.

## Getting Started

### Mod Structure
Each mod lives in its own directory under `mods/`:
```
mods/
  my_mod/
    mod.json          # Mod manifest (required)
    my_mod.dll        # Native plugin (for native mods)
    init.lua          # Lua script (for Lua mods)
    textures/         # Custom block textures (optional)
    README.md         # Mod documentation (optional)
```

### Mod Manifest (mod.json)
Every mod requires a `mod.json` manifest:
```json
{
  "id": "my_mod",
  "name": "My Awesome Mod",
  "version": "1.0.0",
  "author": "YourName",
  "description": "Does awesome things",
  "apiVersion": 1,
  "dependencies": ["other_mod_id"],
  "loadPriority": 100,
  "type": "native" or "lua",
  "entry": "my_mod.dll" or "init.lua"
}
```

**Fields:**
- `id`: Unique mod identifier (alphanumeric + underscores only)
- `name`: Display name
- `version`: Semantic version (X.Y.Z)
- `author`: Author name
- `description`: Mod description
- `apiVersion`: Engine API version (currently 1)
- `dependencies`: Array of mod IDs this mod depends on
- `loadPriority`: Load order (lower = earlier, 0-1000 range)
- `type`: "native" for C++ plugins, "lua" for Lua scripts
- `entry`: Filename of plugin (.dll/.so/.dylib) or script (.lua)

## Native Plugin Development

### Plugin Entry Points
Native plugins must export these C functions:
```cpp
extern "C" {
    ModInfo* GetModInfo();              // Return mod metadata
    bool InitializeMod(const ModAPI* api);  // Initialize mod
    void UpdateMod(float deltaTime);    // Called each server tick
    void ShutdownMod();                 // Cleanup on unload
}
```

### ModAPI Reference
The engine provides a C API struct with function pointers:

#### Block API
- `registerBlock(name, isSolid, isOpaque, isTransparent, textureName, hardness)` - Register custom block, returns block ID
- `getBlockID(name)` - Get block ID by name
- `getBlockName(id, outBuffer, bufferSize)` - Get block name by ID
- `setBlockAt(x, y, z, blockId)` - Place block in world
- `getBlockAt(x, y, z)` - Query block at position

#### Entity API
- `spawnEntity(name, x, y, z)` - Spawn entity, returns entity ID
- `destroyEntity(entityId)` - Remove entity
- `getEntityPosition(entityId, &x, &y, &z)` - Query entity position
- `setEntityPosition(entityId, x, y, z)` - Set entity position

#### Event API
- `subscribeEvent(eventType, callback, userData)` - Subscribe to event, returns subscription ID
- `unsubscribeEvent(subscriptionId)` - Unsubscribe from event
- `publishEvent(eventType, eventData)` - Publish custom event

#### World API
- `getChunkLoaded(chunkX, chunkZ)` - Check if chunk is loaded
- `getWorldSeed()` - Get world generation seed

#### Logging API
- `logInfo(message)` - Log info message
- `logWarn(message)` - Log warning
- `logError(message)` - Log error

#### Config API
- `getConfigInt(key, defaultValue)` - Get integer config value
- `getConfigFloat(key, defaultValue)` - Get float config value
- `getConfigString(key, defaultValue, outBuffer, bufferSize)` - Get string config value
- `setConfigInt(key, value)` - Set integer config value

### Example Native Plugin
See `mods/example_native_mod/example_native_mod.cpp` for a complete example.

```cpp
#include "poorcraft/modding/ModAPI.h"

static ModInfo s_ModInfo = {
    "Example Mod", "1.0.0", "Author", "Description", 1
};

static const ModAPI* s_API = nullptr;

extern "C" {

ModInfo* GetModInfo() {
    return &s_ModInfo;
}

bool InitializeMod(const ModAPI* api) {
    s_API = api;
    s_API->logInfo("My mod initializing...");
    
    // Register custom block
    uint16_t blockId = s_API->registerBlock(
        "my_block", true, true, false, "stone", 2.0f
    );
    
    return true;
}

void UpdateMod(float deltaTime) {
    // Per-tick logic
}

void ShutdownMod() {
    s_API->logInfo("My mod shutting down...");
}

} // extern "C"
```

### Building Native Plugins
See `mods/example_native_mod/CMakeLists.txt` for build configuration example.

```bash
cd mods/my_mod
mkdir build && cd build
cmake ..
cmake --build .
```

## Lua Script Development

### Lua API Reference
Lua scripts have access to these global namespaces:

#### Global Variables
- `ENGINE_VERSION` - Engine version string ("0.1.0")
- `API_VERSION` - Mod API version number (1)

#### Block Namespace
```lua
Block.registerBlock({name, isSolid, isOpaque, textureName, hardness})
Block.getBlockID(name)
Block.getBlockName(id)
Block.setBlockAt(x, y, z, blockId)
Block.getBlockAt(x, y, z)
```

#### Entity Namespace
```lua
Entity.spawn(name, x, y, z)
Entity.destroy(entityId)
Entity.getPosition(entityId)
Entity.setPosition(entityId, x, y, z)
```

#### EventBus Namespace
```lua
EventBus.subscribe(eventType, callback)
EventBus.publish(eventType, eventData)
```

#### World Namespace
```lua
World.getBlockAt(x, y, z)
World.setBlockAt(x, y, z, blockId)
World.getSeed()
World.getChunkLoaded(chunkX, chunkZ)
```

#### Config Namespace
```lua
Config.getInt(key, defaultValue)
Config.getFloat(key, defaultValue)
Config.getString(key, defaultValue)
Config.setInt(key, value)
```

### Event Types
Available event types for subscription:
- `PlayerJoined` - Player connects to server
- `PlayerLeft` - Player disconnects
- `BlockPlaced` - Block placed in world
- `BlockBroken` - Block broken in world
- `EntitySpawned` - Entity spawned
- `EntityDestroyed` - Entity destroyed
- `ChunkGenerated` - Chunk generated

### Optional Update Function
Define global `update(deltaTime)` function for per-tick logic:
```lua
function update(deltaTime)
    -- Called each server tick (60Hz)
end
```

### Example Lua Mod
See `mods/example_lua_mod/init.lua` for a complete example.

```lua
print("My mod loading...")

-- Register custom block
local blockId = Block.registerBlock({
    name = "my_block",
    isSolid = true,
    textureName = "stone",
    hardness = 2.0
})

-- Subscribe to events
EventBus.subscribe("PlayerJoined", function(event)
    print("Welcome " .. event.playerName .. "!")
end)

-- Optional update function
function update(deltaTime)
    -- Per-tick logic
end

print("My mod loaded!")
```

## Hot-Reload
In development builds, mods are automatically reloaded when files change:
- **Native plugins**: Recompile .dll/.so/.dylib, engine detects change and reloads
- **Lua scripts**: Edit .lua file, engine detects change and re-executes

**Note**: Hot-reload calls `ShutdownMod()` before unloading, ensure proper cleanup.

## Best Practices
- Use `extern "C"` for all native plugin exports to avoid name mangling
- Handle errors gracefully (return false from `InitializeMod` on failure)
- Clean up resources in `ShutdownMod` (unsubscribe events, destroy entities)
- Use load priority to control mod load order (lower = earlier)
- Declare dependencies in mod.json for automatic load ordering
- Test mods in singleplayer before deploying to multiplayer servers
- Use Config API for mod settings (`Mods.my_mod.setting_name`)
- Keep `UpdateMod()` lightweight - called at 60Hz

## Configuration
Mods can be configured in `config.ini`:

```ini
[Mods]
enable_mods=true
mods_directory=mods
enable_hot_reload=true

# Per-mod enable/disable
my_mod.enabled=true
```

## Example Mods
- `mods/example_native_mod/` - Native plugin demonstrating block registration
- `mods/example_lua_mod/` - Lua script demonstrating entity spawning
- `mods/event_logger_mod/` - Lua script demonstrating event monitoring

## Troubleshooting
- **Mod not loading**: Check mod.json syntax, ensure apiVersion matches engine
- **Symbol not found**: Ensure `extern "C"` on native plugin exports
- **Lua errors**: Check Lua syntax, ensure API calls match documentation
- **Hot-reload not working**: Check file permissions, ensure debug build
- **Dependency errors**: Check dependency IDs match mod.json id fields

## API Version History
- **Version 1** (Current): Initial modding API release

For more details, see `docs/API.md` for complete API reference.

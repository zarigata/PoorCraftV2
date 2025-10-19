# PoorCraft Mods Directory

This directory contains server-side mods for PoorCraft. Each mod lives in its own subdirectory with a `mod.json` manifest.

## Directory Structure
```
mods/
  example_native_mod/     # Example C++ plugin
    mod.json
    example_native_mod.dll/.so/.dylib
    CMakeLists.txt
    example_native_mod.cpp
  example_lua_mod/        # Example Lua script
    mod.json
    init.lua
  event_logger_mod/       # Event monitoring script
    mod.json
    logger.lua
  your_mod/               # Your custom mod
    mod.json
    ...
```

## Installing Mods

1. **Download or create mod**: Get mod files (mod.json + plugin/script)
2. **Create mod directory**: `mods/<mod_id>/` 
3. **Copy mod files**: Place mod.json and plugin/script in directory
4. **Enable mod**: Edit `config.ini` [Mods] section or set `Mods.<mod_id>.enabled=true` 
5. **Restart server**: Or use hot-reload in debug builds

## Creating Mods

See `docs/MODDING.md` for complete modding guide.

### Quick Start (Lua)

1. Create directory: `mods/my_mod/` 
2. Create `mod.json` with metadata
3. Create `init.lua` with mod logic:
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
```

### Quick Start (Native)

1. Create directory: `mods/my_mod/` 
2. Create `mod.json` with metadata (type="native")
3. Create `my_mod.cpp` with plugin code (see example_native_mod)
4. Create `CMakeLists.txt` for building
5. Build plugin: `cd mods/my_mod && mkdir build && cd build && cmake .. && cmake --build .` 
6. Copy output to `mods/my_mod/` 

## Example Mods

- **example_native_mod**: Demonstrates native plugin with block registration and event hooks
- **example_lua_mod**: Demonstrates Lua script with entity spawning on player join
- **event_logger_mod**: Logs all gameplay events for server monitoring

## Configuration

Edit `config.ini` [Mods] section:
```ini
[Mods]
enable_mods=true
mods_directory=mods
enable_hot_reload=true

# Per-mod enable/disable
my_mod.enabled=true
```

## Troubleshooting

- **Mod not loading**: Check mod.json syntax, ensure apiVersion=1, check logs for errors
- **Native plugin errors**: Ensure extern "C" on exports, check library dependencies
- **Lua errors**: Check Lua syntax, ensure API calls match documentation
- **Hot-reload not working**: Ensure debug build, check file permissions

For more help, see `docs/MODDING.md` or join the community Discord.

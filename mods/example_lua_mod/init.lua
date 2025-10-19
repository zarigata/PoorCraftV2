-- Example Lua Mod for PoorCraft
-- Demonstrates entity spawning and event hooks

print("Example Lua Mod loading...")
print("Engine Version: " .. ENGINE_VERSION)
print("API Version: " .. API_VERSION)

-- Register a custom block
local customBlockId = Block.registerBlock({
    name = "lua_custom_block",
    isSolid = true,
    isOpaque = true,
    isTransparent = false,
    textureName = "stone",  -- Reuse stone texture
    hardness = 3.0
})

print("Registered custom block with ID: " .. customBlockId)

-- Subscribe to player join event
EventBus.subscribe("PlayerJoined", function(event)
    print("Player joined: " .. (event.playerName or "Unknown"))
    
    -- Spawn a friendly entity near the player (example)
    -- Note: This would need actual player position from event
    local entityId = Entity.spawn("WelcomeEntity", 0, 64, 0)
    print("Spawned welcome entity with ID: " .. entityId)
end)

-- Subscribe to block placed event
EventBus.subscribe("BlockPlaced", function(event)
    print("Block placed at (" .. (event.x or 0) .. ", " .. (event.y or 0) .. ", " .. (event.z or 0) .. ")")
    
    -- If player placed our custom block, log special message
    if event.blockId == customBlockId then
        print("Player placed our custom Lua block!")
    end
end)

-- Optional: Define update function called each server tick
function update(deltaTime)
    -- Per-tick logic here (e.g., update custom entities)
    -- Keep this lightweight - called at 60Hz
end

print("Example Lua Mod loaded successfully!")

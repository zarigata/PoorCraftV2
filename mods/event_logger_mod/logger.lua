-- Event Logger Mod
-- Logs all gameplay events for server monitoring and debugging

print("[EventLogger] Event Logger Mod v1.0.0 loading...")

local stats = {
    blocksPlaced = 0,
    blocksBroken = 0,
    entitiesSpawned = 0,
    playersJoined = 0,
    playersLeft = 0
}

-- Log all block placements
EventBus.subscribe("BlockPlaced", function(event)
    stats.blocksPlaced = stats.blocksPlaced + 1
    local blockName = Block.getBlockName(event.blockId or 0) or "Unknown"
    print("[EventLogger] BLOCK_PLACED: " .. blockName .. " at (" .. 
          (event.x or 0) .. ", " .. (event.y or 0) .. ", " .. (event.z or 0) .. 
          ") by player " .. (event.playerId or 0))
end)

-- Log all block breaks
EventBus.subscribe("BlockBroken", function(event)
    stats.blocksBroken = stats.blocksBroken + 1
    local blockName = Block.getBlockName(event.blockId or 0) or "Unknown"
    print("[EventLogger] BLOCK_BROKEN: " .. blockName .. " at (" .. 
          (event.x or 0) .. ", " .. (event.y or 0) .. ", " .. (event.z or 0) .. 
          ") by player " .. (event.playerId or 0))
end)

-- Log entity spawns
EventBus.subscribe("EntitySpawned", function(event)
    stats.entitiesSpawned = stats.entitiesSpawned + 1
    print("[EventLogger] ENTITY_SPAWNED: " .. (event.entityName or "Unknown") .. 
          " (ID: " .. (event.entityId or 0) .. ")")
end)

-- Log player connections
EventBus.subscribe("PlayerJoined", function(event)
    stats.playersJoined = stats.playersJoined + 1
    print("[EventLogger] PLAYER_JOINED: " .. (event.playerName or "Unknown") .. 
          " (ID: " .. (event.playerId or 0) .. ")")
end)

EventBus.subscribe("PlayerLeft", function(event)
    stats.playersLeft = stats.playersLeft + 1
    print("[EventLogger] PLAYER_LEFT: " .. (event.playerName or "Unknown") .. 
          " - " .. (event.reason or "Unknown"))
end)

-- Print statistics every 5 minutes
local statsInterval = 300.0  -- 5 minutes
local statsTimer = 0.0

function update(deltaTime)
    statsTimer = statsTimer + deltaTime
    if statsTimer >= statsInterval then
        print("[EventLogger] ===== Statistics (last 5 min) =====")
        print("  Blocks placed: " .. stats.blocksPlaced)
        print("  Blocks broken: " .. stats.blocksBroken)
        print("  Entities spawned: " .. stats.entitiesSpawned)
        print("  Players joined: " .. stats.playersJoined)
        print("  Players left: " .. stats.playersLeft)
        print("[EventLogger] ===============================")
        statsTimer = 0.0
    end
end

print("[EventLogger] Event logger initialized. Monitoring all gameplay events.")

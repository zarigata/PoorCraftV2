package com.poorcraft.api.event.entity;

import com.poorcraft.api.event.Event;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.world.World;

public class EntitySpawnEvent extends Event implements Event.Cancellable {
    public enum SpawnReason {
        NATURAL,
        SPAWNER,
        COMMAND,
        MOD,
        UNKNOWN
    }

    private final Entity entity;
    private final World world;
    private final SpawnReason reason;

    public EntitySpawnEvent(Entity entity, World world, SpawnReason reason) {
        this.entity = entity;
        this.world = world;
        this.reason = reason == null ? SpawnReason.UNKNOWN : reason;
    }

    public Entity getEntity() {
        return entity;
    }

    public World getWorld() {
        return world;
    }

    public SpawnReason getReason() {
        return reason;
    }
}

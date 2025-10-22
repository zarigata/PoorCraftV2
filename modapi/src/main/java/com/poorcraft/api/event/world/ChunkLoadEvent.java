package com.poorcraft.api.event.world;

import com.poorcraft.api.event.Event;
import com.poorcraft.common.world.World;
import com.poorcraft.common.world.chunk.Chunk;

import java.util.Objects;

public class ChunkLoadEvent extends Event {
    private final Chunk chunk;
    private final World world;
    private final boolean newChunk;

    public ChunkLoadEvent(Chunk chunk, World world, boolean newChunk) {
        this.chunk = Objects.requireNonNull(chunk, "chunk");
        this.world = Objects.requireNonNull(world, "world");
        this.newChunk = newChunk;
    }

    public Chunk getChunk() {
        return chunk;
    }

    public World getWorld() {
        return world;
    }

    public boolean isNewChunk() {
        return newChunk;
    }
}

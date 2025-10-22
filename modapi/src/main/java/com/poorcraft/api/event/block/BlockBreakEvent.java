package com.poorcraft.api.event.block;

import com.poorcraft.api.event.Event;
import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.inventory.ItemStack;
import com.poorcraft.common.world.World;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class BlockBreakEvent extends Event implements Event.Cancellable {
    private final World world;
    private final int x;
    private final int y;
    private final int z;
    private int blockId;
    private final Entity player;
    private final List<ItemStack> drops;

    public BlockBreakEvent(World world, int x, int y, int z, int blockId, Entity player, List<ItemStack> drops) {
        this.world = world;
        this.x = x;
        this.y = y;
        this.z = z;
        this.blockId = blockId;
        this.player = player;
        this.drops = new ArrayList<>(drops);
    }

    public World getWorld() {
        return world;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public int getZ() {
        return z;
    }

    public int getBlockId() {
        return blockId;
    }

    public void setBlockId(int blockId) {
        this.blockId = blockId;
    }

    public Entity getPlayer() {
        return player;
    }

    public List<ItemStack> getDrops() {
        return Collections.unmodifiableList(drops);
    }

    public void setDrops(List<ItemStack> drops) {
        this.drops.clear();
        this.drops.addAll(drops);
    }
}

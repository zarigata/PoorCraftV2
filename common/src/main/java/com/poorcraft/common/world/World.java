package com.poorcraft.common.world;

public interface World {

    int getBlock(int x, int y, int z);

    void setBlock(int x, int y, int z, int blockId);
}

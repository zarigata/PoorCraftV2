package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;

public class InteractionComponent implements Component {

    private boolean breakingBlock;
    private int breakBlockX;
    private int breakBlockY;
    private int breakBlockZ;
    private float breakProgress;
    private long breakStartTime;
    private float targetBreakTime;
    private long lastPlaceTime;

    public InteractionComponent() {
        this.breakingBlock = false;
        this.breakProgress = 0.0f;
        this.breakStartTime = 0L;
        this.targetBreakTime = 0.0f;
        this.lastPlaceTime = 0L;
    }

    public void startBreaking(int x, int y, int z, float breakTime) {
        this.breakingBlock = true;
        this.breakBlockX = x;
        this.breakBlockY = y;
        this.breakBlockZ = z;
        this.breakProgress = 0.0f;
        this.breakStartTime = System.currentTimeMillis();
        this.targetBreakTime = breakTime;
    }

    public void updateBreaking(float dt) {
        if (!breakingBlock) {
            return;
        }
        if (targetBreakTime <= 0.0f) {
            breakProgress = 1.0f;
            return;
        }
        breakProgress = Math.min(1.0f, breakProgress + dt / targetBreakTime);
    }

    public void cancelBreaking() {
        this.breakingBlock = false;
        this.breakProgress = 0.0f;
        this.breakStartTime = 0L;
        this.targetBreakTime = 0.0f;
    }

    public boolean isBreaking() {
        return breakingBlock;
    }

    public float getBreakProgress() {
        return breakProgress;
    }

    public int getBreakBlockX() {
        return breakBlockX;
    }

    public int getBreakBlockY() {
        return breakBlockY;
    }

    public int getBreakBlockZ() {
        return breakBlockZ;
    }

    public long getBreakStartTime() {
        return breakStartTime;
    }

    public float getTargetBreakTime() {
        return targetBreakTime;
    }

    public boolean canPlace(long now, float delaySeconds) {
        long delayMillis = (long) (delaySeconds * 1000.0f);
        return now - lastPlaceTime >= delayMillis;
    }

    public void recordPlace(long now) {
        this.lastPlaceTime = now;
    }
}

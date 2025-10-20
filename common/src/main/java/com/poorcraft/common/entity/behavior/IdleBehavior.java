package com.poorcraft.common.entity.behavior;

import com.poorcraft.common.entity.Entity;

import java.util.Random;

public class IdleBehavior implements BehaviorState {
    private static final Random RANDOM = new Random();
    private float idleTime;
    private float maxIdleTime;

    public IdleBehavior() {
        this.maxIdleTime = 5.0f;
    }

    @Override
    public void onEnter(Entity entity) {
        idleTime = 0.0f;
    }

    @Override
    public void onUpdate(Entity entity, double dt) {
        idleTime += dt;
        if (idleTime >= maxIdleTime) {
            float newMax = 3.0f + RANDOM.nextFloat() * 5.0f;
            maxIdleTime = newMax;
            idleTime = 0.0f;
        }
    }

    @Override
    public void onExit(Entity entity) {
    }

    @Override
    public boolean canTransition(Entity entity, String targetState) {
        return "wander".equals(targetState) || "follow".equals(targetState) || "idle".equals(targetState);
    }
}

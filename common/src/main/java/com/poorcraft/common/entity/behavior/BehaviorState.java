package com.poorcraft.common.entity.behavior;

import com.poorcraft.common.entity.Entity;

public interface BehaviorState {
    void onEnter(Entity entity);

    void onUpdate(Entity entity, double dt);

    void onExit(Entity entity);

    boolean canTransition(Entity entity, String targetState);
}

package com.poorcraft.common.entity.behavior;

import com.poorcraft.common.entity.Entity;
import com.poorcraft.common.entity.component.BehaviorComponent;

import java.util.HashMap;
import java.util.Map;

public class BehaviorStateMachine {
    private final Entity entity;
    private final Map<String, BehaviorState> states;
    private BehaviorState currentState;
    private String currentStateName;

    public BehaviorStateMachine(Entity entity) {
        this.entity = entity;
        this.states = new HashMap<>();
    }

    public void registerState(String name, BehaviorState state) {
        if (name != null && state != null) {
            states.put(name, state);
        }
    }

    public void setState(String name) {
        if (name == null) {
            return;
        }
        BehaviorState next = states.get(name);
        if (next == null) {
            return;
        }
        BehaviorComponent behavior = entity.getComponent(BehaviorComponent.class);
        if (currentState != null) {
            if (!currentState.canTransition(entity, name)) {
                return;
            }
            currentState.onExit(entity);
        }
        currentState = next;
        currentStateName = name;
        currentState.onEnter(entity);
        if (behavior != null) {
            behavior.setState(name);
        }
    }

    public void update(double dt) {
        if (currentState != null) {
            currentState.onUpdate(entity, dt);
        }
        BehaviorComponent behavior = entity.getComponent(BehaviorComponent.class);
        if (behavior != null) {
            behavior.onBehaviorUpdate(dt);
            String requestedState = behavior.getCurrentState();
            if (requestedState != null && !requestedState.equals(currentStateName)) {
                setState(requestedState);
            }
        }
    }

    public boolean canTransitionTo(String targetState) {
        if (currentState == null) {
            return true;
        }
        return currentState.canTransition(entity, targetState);
    }

    public String getCurrentStateName() {
        return currentStateName;
    }

    public Entity getEntity() {
        return entity;
    }
}

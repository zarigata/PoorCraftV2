package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Holds behaviour metadata and state for NPC entities.
 */
public class BehaviorComponent implements Component {
    private String behaviorType;
    private final Map<String, Object> behaviorData;
    private boolean aiEnabled;
    private boolean dialogueEnabled;
    private String currentState;
    private final Map<String, Object> stateData;

    public BehaviorComponent(String behaviorType) {
        this.behaviorType = behaviorType;
        this.behaviorData = new HashMap<>();
        this.stateData = new HashMap<>();
        this.aiEnabled = true;
        this.dialogueEnabled = false;
        this.currentState = "idle";
    }

    public String getBehaviorType() {
        return behaviorType;
    }

    public void setBehaviorType(String behaviorType) {
        this.behaviorType = behaviorType;
    }

    public boolean isAIEnabled() {
        return aiEnabled;
    }

    public void setAIEnabled(boolean aiEnabled) {
        this.aiEnabled = aiEnabled;
    }

    public boolean isDialogueEnabled() {
        return dialogueEnabled;
    }

    public void setDialogueEnabled(boolean dialogueEnabled) {
        this.dialogueEnabled = dialogueEnabled;
    }

    public String getCurrentState() {
        return currentState;
    }

    public void setState(String state) {
        this.currentState = state;
    }

    public Map<String, Object> getBehaviorData() {
        return Collections.unmodifiableMap(behaviorData);
    }

    public void setBehaviorData(String key, Object value) {
        behaviorData.put(key, value);
    }

    public Object getBehaviorValue(String key) {
        return behaviorData.get(key);
    }

    public Map<String, Object> getStateData() {
        return Collections.unmodifiableMap(stateData);
    }

    public void setStateData(String key, Object value) {
        stateData.put(key, value);
    }

    public Object getStateValue(String key) {
        return stateData.get(key);
    }

    public String onDialogueRequest(String input) {
        return "Dialogue not implemented";
    }

    public void onBehaviorUpdate(double dt) {
        // Placeholder for custom behaviour logic
    }
}

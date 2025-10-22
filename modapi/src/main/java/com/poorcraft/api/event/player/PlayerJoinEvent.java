package com.poorcraft.api.event.player;

import com.poorcraft.api.event.Event;
import com.poorcraft.common.entity.Entity;

public class PlayerJoinEvent extends Event {
    private final Entity player;
    private String joinMessage;

    public PlayerJoinEvent(Entity player, String joinMessage) {
        this.player = player;
        this.joinMessage = joinMessage;
    }

    public Entity getPlayer() {
        return player;
    }

    public String getJoinMessage() {
        return joinMessage;
    }

    public void setJoinMessage(String joinMessage) {
        this.joinMessage = joinMessage;
    }
}

package com.poorcraft.common.entity;

import java.util.Arrays;

public enum EntityType {
    PLAYER(0, "player", 0.6f, 1.8f, 1.62f, false),
    NPC(1, "npc", 0.6f, 1.8f, 1.62f, true),
    MOB(2, "mob", 0.9f, 0.9f, 0.8f, true);

    private static final EntityType[] LOOKUP = Arrays.stream(values())
            .sorted((a, b) -> Integer.compare(a.id, b.id))
            .toArray(EntityType[]::new);

    private final int id;
    private final String name;
    private final float width;
    private final float height;
    private final float eyeHeight;
    private final boolean hasAI;

    EntityType(int id, String name, float width, float height, float eyeHeight, boolean hasAI) {
        this.id = id;
        this.name = name;
        this.width = width;
        this.height = height;
        this.eyeHeight = eyeHeight;
        this.hasAI = hasAI;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public float getWidth() {
        return width;
    }

    public float getHeight() {
        return height;
    }

    public float getEyeHeight() {
        return eyeHeight;
    }

    public boolean hasAI() {
        return hasAI;
    }

    public static EntityType getById(int id) {
        if (id < 0 || id >= LOOKUP.length) {
            return null;
        }
        return LOOKUP[id];
    }
}

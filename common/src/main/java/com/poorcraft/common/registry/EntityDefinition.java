package com.poorcraft.common.registry;

import com.poorcraft.common.entity.EntityType;
import com.poorcraft.common.registry.Registry.RegistryObject;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class EntityDefinition implements RegistryObject {
    private final String id;
    private final String name;
    private final float width;
    private final float height;
    private final float eyeHeight;
    private final boolean hasAI;
    private final float maxHealth;
    private final Map<String, Object> properties;
    private int numericId = -1;

    private EntityDefinition(Builder builder) {
        this.id = Objects.requireNonNull(builder.id, "id");
        this.name = Objects.requireNonNull(builder.name, "name");
        this.width = builder.width;
        this.height = builder.height;
        this.eyeHeight = builder.eyeHeight;
        this.hasAI = builder.hasAI;
        this.maxHealth = builder.maxHealth;
        this.properties = Collections.unmodifiableMap(new HashMap<>(builder.properties));
    }

    public String getId() {
        return id;
    }

    @Override
    public void setNumericId(int id) {
        this.numericId = id;
    }

    @Override
    public int getNumericId() {
        return numericId;
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

    public float getMaxHealth() {
        return maxHealth;
    }

    public Map<String, Object> getProperties() {
        return properties;
    }

    public static EntityDefinition fromEntityType(EntityType entityType) {
        Builder builder = new Builder("minecraft:" + entityType.getName())
                .name(entityType.getName())
                .dimensions(entityType.getWidth(), entityType.getHeight())
                .eyeHeight(entityType.getEyeHeight())
                .ai(entityType.hasAI())
                .maxHealth(20f);
        return builder.build();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof EntityDefinition)) return false;
        EntityDefinition that = (EntityDefinition) o;
        return id.equals(that.id);
    }

    @Override
    public int hashCode() {
        return id.hashCode();
    }

    public static Builder builder(String id) {
        return new Builder(id);
    }

    public static class Builder {
        private final String id;
        private String name;
        private float width = 0.6f;
        private float height = 1.8f;
        private float eyeHeight = 1.62f;
        private boolean hasAI;
        private float maxHealth = 20f;
        private final Map<String, Object> properties = new HashMap<>();

        private Builder(String id) {
            this.id = id;
        }

        public Builder name(String name) {
            this.name = name;
            return this;
        }

        public Builder dimensions(float width, float height) {
            this.width = width;
            this.height = height;
            return this;
        }

        public Builder eyeHeight(float eyeHeight) {
            this.eyeHeight = eyeHeight;
            return this;
        }

        public Builder ai(boolean hasAI) {
            this.hasAI = hasAI;
            return this;
        }

        public Builder maxHealth(float maxHealth) {
            this.maxHealth = maxHealth;
            return this;
        }

        public Builder property(String key, Object value) {
            this.properties.put(key, value);
            return this;
        }

        public EntityDefinition build() {
            return new EntityDefinition(this);
        }
    }
}

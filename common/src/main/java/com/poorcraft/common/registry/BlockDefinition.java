package com.poorcraft.common.registry;

import com.poorcraft.common.registry.Registry.RegistryObject;
import com.poorcraft.common.world.block.BlockType;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class BlockDefinition implements RegistryObject {
    private final String id;
    private final String name;
    private final boolean transparent;
    private final boolean solid;
    private final boolean liquid;
    private final float hardness;
    private final float resistance;
    private final Map<String, String> textures;
    private final Map<String, Object> properties;
    private int numericId = -1;

    private BlockDefinition(Builder builder) {
        this.id = Objects.requireNonNull(builder.id, "id");
        this.name = Objects.requireNonNull(builder.name, "name");
        this.transparent = builder.transparent;
        this.solid = builder.solid;
        this.liquid = builder.liquid;
        this.hardness = builder.hardness;
        this.resistance = builder.resistance;
        this.textures = Collections.unmodifiableMap(new HashMap<>(builder.textures));
        this.properties = Collections.unmodifiableMap(new HashMap<>(builder.properties));
    }

    public String getId() {
        return id;
    }

    public int getNumericId() {
        return numericId;
    }

    @Override
    public void setNumericId(int id) {
        this.numericId = id;
    }

    public String getName() {
        return name;
    }

    public boolean isTransparent() {
        return transparent;
    }

    public boolean isSolid() {
        return solid;
    }

    public boolean isLiquid() {
        return liquid;
    }

    public float getHardness() {
        return hardness;
    }

    public float getResistance() {
        return resistance;
    }

    public Map<String, String> getTextures() {
        return textures;
    }

    public Map<String, Object> getProperties() {
        return properties;
    }

    public String getTexture(String face) {
        return textures.get(face);
    }

    public static BlockDefinition fromBlockType(BlockType blockType) {
        Builder builder = new Builder("minecraft:" + blockType.getName())
                .name(blockType.getName())
                .transparent(blockType.isTransparent())
                .solid(blockType.isSolid())
                .liquid(blockType.isLiquid())
                .hardness(1.0f)
                .resistance(1.0f);

        builder.texture("top", blockType.getTextureName(1));
        builder.texture("bottom", blockType.getTextureName(0));
        builder.texture("side", blockType.getTextureName(2));
        return builder.build();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof BlockDefinition)) return false;
        BlockDefinition that = (BlockDefinition) o;
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
        private boolean transparent;
        private boolean solid = true;
        private boolean liquid;
        private float hardness = 1.0f;
        private float resistance = 1.0f;
        private final Map<String, String> textures = new HashMap<>();
        private final Map<String, Object> properties = new HashMap<>();

        private Builder(String id) {
            this.id = id;
        }

        public Builder name(String name) {
            this.name = name;
            return this;
        }

        public Builder transparent(boolean transparent) {
            this.transparent = transparent;
            return this;
        }

        public Builder solid(boolean solid) {
            this.solid = solid;
            return this;
        }

        public Builder liquid(boolean liquid) {
            this.liquid = liquid;
            return this;
        }

        public Builder hardness(float hardness) {
            this.hardness = hardness;
            return this;
        }

        public Builder resistance(float resistance) {
            this.resistance = resistance;
            return this;
        }

        public Builder texture(String face, String texture) {
            this.textures.put(face, texture);
            return this;
        }

        public Builder property(String key, Object value) {
            this.properties.put(key, value);
            return this;
        }

        public BlockDefinition build() {
            return new BlockDefinition(this);
        }
    }
}

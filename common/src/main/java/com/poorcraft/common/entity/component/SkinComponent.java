package com.poorcraft.common.entity.component;

import com.poorcraft.common.Constants;
import com.poorcraft.common.entity.Component;

/**
 * Holds skin/texture information for an entity.
 */
public class SkinComponent implements Component {
    private String skinPath;
    private final int skinWidth;
    private final int skinHeight;
    private boolean loaded;
    private boolean slim;

    public SkinComponent(String skinPath) {
        this.skinPath = skinPath;
        this.skinWidth = Constants.Skin.TEXTURE_WIDTH;
        this.skinHeight = Constants.Skin.TEXTURE_HEIGHT;
        this.loaded = false;
        this.slim = false;
    }

    public String getSkinPath() {
        return skinPath;
    }

    public void setSkinPath(String skinPath) {
        this.skinPath = skinPath;
        this.loaded = false;
    }

    public int getSkinWidth() {
        return skinWidth;
    }

    public int getSkinHeight() {
        return skinHeight;
    }

    public boolean isLoaded() {
        return loaded;
    }

    public void setLoaded(boolean loaded) {
        this.loaded = loaded;
    }

    public boolean isSlim() {
        return slim;
    }

    public void setSlim(boolean slim) {
        this.slim = slim;
    }
}

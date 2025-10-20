package com.poorcraft.common.entity.component;

import com.poorcraft.common.entity.Component;

/**
 * Stores a display name and related metadata for an entity.
 */
public class NameComponent implements Component {
    private String name;
    private boolean visible;
    private int color;

    public NameComponent(String name) {
        this.name = name;
        this.visible = true;
        this.color = 0xFFFFFF;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public boolean isVisible() {
        return visible;
    }

    public void setVisible(boolean visible) {
        this.visible = visible;
    }

    public int getColor() {
        return color;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public String getFormattedName() {
        return String.format("#%06X %s", color & 0xFFFFFF, name);
    }
}

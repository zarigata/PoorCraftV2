package com.poorcraft.client.ui;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.layout.Anchor;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class Widget {
    private float localX;
    private float localY;
    private float absoluteX;
    private float absoluteY;
    private float width;
    private float height;
    private boolean visible;
    private boolean enabled;
    private Widget parent;
    private final List<Widget> children;
    private Anchor anchor;
    private float offsetX;
    private float offsetY;

    protected Widget(float x, float y, float width, float height) {
        this.localX = x;
        this.localY = y;
        this.width = width;
        this.height = height;
        this.visible = true;
        this.enabled = true;
        this.children = new ArrayList<>();
        this.anchor = Anchor.TOP_LEFT;
    }

    public void addChild(Widget child) {
        if (child == null) {
            return;
        }
        child.setParent(this);
        children.add(child);
    }

    public void removeChild(Widget child) {
        if (child == null) {
            return;
        }
        if (children.remove(child)) {
            child.setParent(null);
        }
    }

    public List<Widget> getChildren() {
        return Collections.unmodifiableList(children);
    }

    public Widget getParent() {
        return parent;
    }

    private void setParent(Widget parent) {
        this.parent = parent;
    }

    public Anchor getAnchor() {
        return anchor;
    }

    public void setAnchor(Anchor anchor, float offsetX, float offsetY) {
        if (anchor == null) {
            return;
        }
        this.anchor = anchor;
        this.offsetX = offsetX;
        this.offsetY = offsetY;
    }

    public float getOffsetX() {
        return offsetX;
    }

    public float getOffsetY() {
        return offsetY;
    }

    public float getLocalX() {
        return localX;
    }

    public float getLocalY() {
        return localY;
    }

    public float getX() {
        return absoluteX;
    }

    public float getY() {
        return absoluteY;
    }

    public void setPosition(float x, float y) {
        this.localX = x;
        this.localY = y;
    }

    public void setSize(float width, float height) {
        this.width = width;
        this.height = height;
    }

    public float getWidth() {
        return width;
    }

    public float getHeight() {
        return height;
    }

    public boolean isVisible() {
        return visible;
    }

    public void setVisible(boolean visible) {
        this.visible = visible;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public void setAbsolutePosition(float x, float y) {
        this.absoluteX = x;
        this.absoluteY = y;
        float parentX = parent != null ? parent.getX() : 0.0f;
        float parentY = parent != null ? parent.getY() : 0.0f;
        this.localX = x - parentX;
        this.localY = y - parentY;
    }

    public void setAbsoluteX(float x) {
        setAbsolutePosition(x, this.absoluteY);
    }

    public void setAbsoluteY(float y) {
        setAbsolutePosition(this.absoluteX, y);
    }

    public boolean contains(float x, float y) {
        if (!visible) {
            return false;
        }
        return x >= absoluteX && y >= absoluteY && x <= absoluteX + width && y <= absoluteY + height;
    }

    public void updateLayout(int parentWidth, int parentHeight) {
        float anchorX = anchor.getAnchorX();
        float anchorY = anchor.getAnchorY();
        float parentAbsX = parent != null ? parent.getX() : 0.0f;
        float parentAbsY = parent != null ? parent.getY() : 0.0f;
        float baseX = parentWidth * anchorX - width * anchorX;
        float baseY = parentHeight * anchorY - height * anchorY;
        this.absoluteX = parentAbsX + baseX + offsetX + localX;
        this.absoluteY = parentAbsY + baseY + offsetY + localY;
        for (Widget child : children) {
            child.updateLayout(Math.round(width), Math.round(height));
        }
    }

    public abstract void render(UIRenderer renderer, float mouseX, float mouseY);

    public abstract boolean handleInput(InputManager input, float mouseX, float mouseY);
}

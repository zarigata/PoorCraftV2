package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.client.ui.util.ScissorStack;
import java.util.List;

public class Panel extends Widget {
    private int backgroundColor;
    private int borderColor;
    private float borderWidth;
    private float padding;

    public Panel(float x, float y, float width, float height) {
        super(x, y, width, height);
        this.backgroundColor = Color.rgba(32, 32, 32, 200);
        this.borderColor = Color.rgba(255, 255, 255, 64);
        this.borderWidth = 0.0f;
        this.padding = 0.0f;
    }

    public void setBackgroundColor(int color) {
        this.backgroundColor = color;
    }

    public void setBorder(int color, float width) {
        this.borderColor = color;
        this.borderWidth = Math.max(0.0f, width);
    }

    public void setPadding(float padding) {
        this.padding = Math.max(0.0f, padding);
    }

    public float getPadding() {
        return padding;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible()) {
            return;
        }

        float x = getX();
        float y = getY();
        float width = getWidth();
        float height = getHeight();

        // Push scissor area for clipping
        ScissorStack.push((int)x, (int)y, (int)width, (int)height);

        try {
            renderer.drawRect(x, y, width, height, backgroundColor);

            if (borderWidth > 0.0f) {
                renderer.drawRect(x, y, width, borderWidth, borderColor);
                renderer.drawRect(x, y + height - borderWidth, width, borderWidth, borderColor);
                renderer.drawRect(x, y, borderWidth, height, borderColor);
                renderer.drawRect(x + width - borderWidth, y, borderWidth, height, borderColor);
            }

            // Render children (clipped by scissor)
            for (Widget child : getChildren()) {
                child.render(renderer, mouseX, mouseY);
            }
        } finally {
            // Pop scissor area
            ScissorStack.pop();
        }
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        boolean handled = false;
        List<Widget> children = getChildren();

        // Handle children in reverse order (top to bottom)
        for (int i = children.size() - 1; i >= 0; i--) {
            Widget child = children.get(i);
            if (!child.isVisible() || !child.isEnabled()) {
                continue;
            }

            // Only propagate to children that are under the mouse or have focus
            if (child.contains(mouseX, mouseY) || (child instanceof FocusableWidget && ((FocusableWidget) child).hasFocus())) {
                if (child.handleInput(input, mouseX, mouseY)) {
                    handled = true;
                    break;
                }
            }
        }

        return handled;
    }

    /**
     * Interface for widgets that can have focus.
     */
    public interface FocusableWidget {
        boolean hasFocus();
        void setFocus(boolean focus);
    }
}

package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;

import java.util.function.Consumer;

/**
 * Checkbox widget for boolean selection.
 */
public class Checkbox extends Widget implements Panel.FocusableWidget {
    private boolean checked;
    private Consumer<Boolean> onChange;
    private boolean focused;
    private int boxColor;
    private int checkColor;
    private int hoverColor;

    public Checkbox(float x, float y, float width, float height, String text) {
        super(x, y, width, height);
        this.checked = false;
        this.focused = false;
        this.boxColor = Color.rgba(100, 100, 100, 200);
        this.checkColor = Color.rgba(0, 150, 0, 255);
        this.hoverColor = Color.rgba(150, 150, 150, 200);
    }

    public Checkbox(float x, float y, float size, boolean initialValue) {
        this(x, y, size, size, "");
        this.checked = initialValue;
    }

    public void setChecked(boolean checked) {
        if (this.checked != checked) {
            this.checked = checked;
            notifyChange();
        }
    }

    public boolean isChecked() {
        return checked;
    }

    public void setOnChange(Consumer<Boolean> onChange) {
        this.onChange = onChange;
    }

    public void toggle() {
        setChecked(!checked);
    }

    @Override
    public boolean hasFocus() {
        return focused;
    }

    @Override
    public void setFocus(boolean focused) {
        this.focused = focused;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible()) {
            return;
        }

        float x = getX();
        float y = getY();
        float size = Math.min(getWidth(), getHeight());

        // Draw checkbox background
        boolean hovered = contains(mouseX, mouseY);
        int bgColor = hovered || focused ? hoverColor : boxColor;
        renderer.drawRect(x, y, size, size, bgColor);

        // Draw border
        renderer.drawRect(x, y, size, 2, Color.BLACK);
        renderer.drawRect(x, y + size - 2, size, 2, Color.BLACK);
        renderer.drawRect(x, y, 2, size, Color.BLACK);
        renderer.drawRect(x + size - 2, y, 2, size, Color.BLACK);

        // Draw check mark if checked
        if (checked) {
            float checkSize = size * 0.6f;
            float checkX = x + (size - checkSize) / 2.0f;
            float checkY = y + (size - checkSize) / 2.0f;
            renderer.drawRect(checkX, checkY, checkSize, checkSize, checkColor);
        }
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        boolean hovered = contains(mouseX, mouseY);

        // Handle mouse click
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (hovered) {
                toggle();
                focused = true;
                return true;
            } else if (focused) {
                focused = false;
                return true;
            }
        }

        // Update focus state
        if (hovered && !focused) {
            focused = true;
        } else if (!hovered && !input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT)) {
            focused = false;
        }

        return false;
    }

    private void notifyChange() {
        if (onChange != null) {
            onChange.accept(checked);
        }
    }
}

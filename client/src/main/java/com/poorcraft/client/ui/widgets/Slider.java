package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;

import java.util.function.Consumer;

/**
 * Slider widget for selecting a value within a range.
 */
public class Slider extends Widget implements Panel.FocusableWidget {
    private float minValue;
    private float maxValue;
    private float value;
    private float step;
    private Consumer<Float> onChange;
    private boolean dragging;
    private boolean focused;
    private int trackColor;
    private int fillColor;
    private int thumbColor;
    private int thumbHoverColor;

    public Slider(float x, float y, float width, float height, float minValue, float maxValue, float initialValue) {
        super(x, y, width, height);
        this.minValue = minValue;
        this.maxValue = maxValue;
        this.value = Math.max(minValue, Math.min(maxValue, initialValue));
        this.step = 0.01f; // Default step
        this.dragging = false;
        this.focused = false;
        this.trackColor = Color.rgba(100, 100, 100, 200);
        this.fillColor = Color.rgba(0, 120, 255, 255);
        this.thumbColor = Color.rgba(200, 200, 200, 255);
        this.thumbHoverColor = Color.rgba(255, 255, 255, 255);
    }

    public void setRange(float minValue, float maxValue) {
        this.minValue = minValue;
        this.maxValue = maxValue;
        this.value = Math.max(minValue, Math.min(maxValue, this.value));
        notifyChange();
    }

    public void setValue(float value) {
        float oldValue = this.value;
        this.value = Math.max(minValue, Math.min(maxValue, value));
        if (Math.abs(this.value - oldValue) > 0.001f) {
            notifyChange();
        }
    }

    public float getValue() {
        return value;
    }

    public void setStep(float step) {
        this.step = Math.max(0.001f, step);
    }

    public void setOnChange(Consumer<Float> onChange) {
        this.onChange = onChange;
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
        float width = getWidth();
        float height = getHeight();

        // Draw track background
        renderer.drawRect(x, y, width, height, trackColor);

        // Calculate fill width based on value
        float normalizedValue = (value - minValue) / (maxValue - minValue);
        float fillWidth = width * normalizedValue;

        if (fillWidth > 0) {
            // Draw fill
            renderer.drawRect(x, y, fillWidth, height, fillColor);
        }

        // Calculate thumb position
        float thumbSize = height * 1.5f;
        float thumbX = x + fillWidth - thumbSize / 2.0f;
        thumbX = Math.max(x, Math.min(x + width - thumbSize, thumbX));

        // Draw thumb
        boolean hovered = contains(mouseX, mouseY);
        int thumbColorToUse = hovered || focused ? thumbHoverColor : thumbColor;
        renderer.drawRect(thumbX, y - (thumbSize - height) / 2.0f, thumbSize, thumbSize, thumbColorToUse);
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        boolean hovered = contains(mouseX, mouseY);

        // Handle mouse press
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (hovered) {
                dragging = true;
                focused = true;
                updateValueFromMouse(mouseX);
                return true;
            } else if (focused) {
                focused = false;
                return true;
            }
        }

        // Handle mouse release
        if (!input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT)) {
            dragging = false;
        }

        // Handle dragging
        if (dragging) {
            updateValueFromMouse(mouseX);
            return true;
        }

        // Update hover state
        if (hovered && !focused) {
            focused = true;
        } else if (!hovered && !dragging) {
            focused = false;
        }

        return false;
    }

    private void updateValueFromMouse(float mouseX) {
        float x = getX();
        float width = getWidth();

        // Calculate normalized position (0.0 to 1.0)
        float normalizedPos = (mouseX - x) / width;
        normalizedPos = Math.max(0.0f, Math.min(1.0f, normalizedPos));

        // Convert to value
        float newValue = minValue + normalizedPos * (maxValue - minValue);

        // Apply step if set
        if (step > 0.001f) {
            newValue = Math.round(newValue / step) * step;
        }

        setValue(newValue);
    }

    private void notifyChange() {
        if (onChange != null) {
            onChange.accept(value);
        }
    }
}

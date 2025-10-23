package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;

public class Button extends Widget {
    private String text;
    private Runnable onClick;
    private int normalColor;
    private int hoverColor;
    private int pressedColor;
    private int textColor;
    private boolean hovered;
    private boolean pressed;

    public Button(float x, float y, float width, float height, String text, Runnable onClick) {
        super(x, y, width, height);
        this.text = text;
        this.onClick = onClick;
        this.normalColor = Color.rgba(60, 60, 60, 220);
        this.hoverColor = Color.rgba(90, 90, 90, 240);
        this.pressedColor = Color.rgba(40, 40, 40, 240);
        this.textColor = Color.WHITE;
        this.hovered = false;
        this.pressed = false;
    }

    public void setText(String text) {
        this.text = text != null ? text : "";
    }

    public void setOnClick(Runnable onClick) {
        this.onClick = onClick;
    }

    public void setColors(int normalColor, int hoverColor, int pressedColor, int textColor) {
        this.normalColor = normalColor;
        this.hoverColor = hoverColor;
        this.pressedColor = pressedColor;
        this.textColor = textColor;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible()) {
            return;
        }

        int color = normalColor;
        if (pressed) {
            color = pressedColor;
        } else if (hovered) {
            color = hoverColor;
        }

        renderer.drawRect(getX(), getY(), getWidth(), getHeight(), color);

        if (text != null && !text.isEmpty()) {
            float textWidth = renderer.measureText(text);
            float textHeight = renderer.getLineHeight();
            float textX = getX() + (getWidth() - textWidth) * 0.5f;
            float textY = getY() + (getHeight() - textHeight) * 0.5f + renderer.getBaselineOffset();
            renderer.drawText(text, textX, textY, textColor);
        }
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        hovered = contains(mouseX, mouseY);
        boolean consumed = false;

        if (hovered && input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            pressed = true;
            consumed = true;
        }

        if (!input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (pressed && hovered && onClick != null) {
                onClick.run();
                consumed = true;
            }
            pressed = false;
        }

        if (!hovered && !input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT)) {
            pressed = false;
        }

        return consumed;
    }
}

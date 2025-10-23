package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;

public class Label extends Widget {
    public enum Alignment {
        LEFT,
        CENTER,
        RIGHT
    }

    private String text;
    private int color;
    private Alignment alignment;
    private boolean shadow;

    public Label(float x, float y, float width, float height, String text) {
        super(x, y, width, height);
        this.text = text != null ? text : "";
        this.color = Color.WHITE;
        this.alignment = Alignment.LEFT;
        this.shadow = false;
    }

    public void setText(String text) {
        this.text = text != null ? text : "";
    }

    public void setColor(int color) {
        this.color = color;
    }

    public void setAlignment(Alignment alignment) {
        if (alignment != null) {
            this.alignment = alignment;
        }
    }

    public void setShadow(boolean shadow) {
        this.shadow = shadow;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible() || text.isEmpty()) {
            return;
        }

        float textWidth = renderer.measureText(text);
        float textHeight = renderer.getLineHeight();
        float x = getX();
        float y = getY() + renderer.getBaselineOffset();

        if (alignment == Alignment.CENTER) {
            x += (getWidth() - textWidth) * 0.5f;
        } else if (alignment == Alignment.RIGHT) {
            x += getWidth() - textWidth;
        }

        if (shadow) {
            renderer.drawText(text, x + 1.0f, y + 1.0f, Color.rgba(0, 0, 0, 128));
        }

        renderer.drawText(text, x, y, color);
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        return false;
    }
}

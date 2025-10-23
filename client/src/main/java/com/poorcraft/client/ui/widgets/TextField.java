package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;

import java.util.function.Consumer;

public class TextField extends Widget {
    private final StringBuilder text;
    private String placeholder;
    private int maxLength;
    private boolean focused;
    private int backgroundColor;
    private int textColor;
    private Consumer<String> onSubmit;

    public TextField(float x, float y, float width, float height, String placeholder) {
        super(x, y, width, height);
        this.text = new StringBuilder();
        this.placeholder = placeholder != null ? placeholder : "";
        this.maxLength = 256;
        this.backgroundColor = Color.rgba(20, 20, 20, 200);
        this.textColor = Color.WHITE;
    }

    public void setPlaceholder(String placeholder) {
        this.placeholder = placeholder != null ? placeholder : "";
    }

    public void setMaxLength(int maxLength) {
        this.maxLength = Math.max(1, maxLength);
    }

    public void setOnSubmit(Consumer<String> onSubmit) {
        this.onSubmit = onSubmit;
    }

    public String getText() {
        return text.toString();
    }

    public void setText(String value) {
        text.setLength(0);
        if (value != null) {
            if (value.length() > maxLength) {
                text.append(value, 0, maxLength);
            } else {
                text.append(value);
            }
        }
    }

    public void clear() {
        text.setLength(0);
    }

    public boolean isFocused() {
        return focused;
    }

    public void setFocused(boolean focused) {
        this.focused = focused;
    }

    @Override
    public void render(UIRenderer renderer, float mouseX, float mouseY) {
        if (!isVisible()) {
            return;
        }

        renderer.drawRect(getX(), getY(), getWidth(), getHeight(), backgroundColor);

        String toRender = text.length() > 0 ? text.toString() : placeholder;
        int color = text.length() > 0 ? textColor : Color.rgba(200, 200, 200, 180);

        float padding = 4.0f;
        float x = getX() + padding;
        float y = getY() + padding + renderer.getBaselineOffset();
        renderer.drawText(toRender, x, y, color);
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            focused = contains(mouseX, mouseY);
            return focused;
        }

        if (!focused) {
            return false;
        }

        String inputText = input.getTextInput();
        if (!inputText.isEmpty()) {
            for (int i = 0; i < inputText.length(); i++) {
                if (text.length() < maxLength) {
                    text.append(inputText.charAt(i));
                }
            }
            return true;
        }

        if (input.keyPressed(org.lwjgl.glfw.GLFW.GLFW_KEY_BACKSPACE) && text.length() > 0) {
            text.deleteCharAt(text.length() - 1);
            return true;
        }

        if (input.keyPressed(org.lwjgl.glfw.GLFW.GLFW_KEY_ENTER)) {
            if (onSubmit != null) {
                onSubmit.accept(text.toString());
            }
            return true;
        }

        return false;
    }
}

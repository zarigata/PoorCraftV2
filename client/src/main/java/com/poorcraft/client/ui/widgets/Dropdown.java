package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.client.ui.util.ScissorStack;
import com.poorcraft.common.Constants;

import java.util.function.Consumer;

/**
 * Dropdown widget for selecting from a list of options.
 */
public class Dropdown extends Widget implements Panel.FocusableWidget {
    private final String[] options;
    private int selectedIndex;
    private boolean expanded;
    private Consumer<Integer> onChange;
    private boolean focused;
    private int backgroundColor;
    private int hoverColor;
    private int textColor;
    private int borderColor;

    public Dropdown(float x, float y, float width, float height, String[] options) {
        super(x, y, width, height);
        this.options = options != null ? options.clone() : new String[0];
        this.selectedIndex = -1;
        this.expanded = false;
        this.focused = false;
        this.backgroundColor = Color.rgba(60, 60, 60, 220);
        this.hoverColor = Color.rgba(90, 90, 90, 240);
        this.textColor = Color.WHITE;
        this.borderColor = Color.rgba(255, 255, 255, 128);
    }

    public Dropdown(float x, float y, float width, float height, String[] options, int initialSelection) {
        this(x, y, width, height, options);
        this.selectedIndex = Math.max(-1, Math.min(options.length - 1, initialSelection));
    }

    public void setSelectedIndex(int index) {
        if (index != selectedIndex && index >= -1 && index < options.length) {
            int oldIndex = selectedIndex;
            selectedIndex = index;
            if (oldIndex != selectedIndex) {
                notifyChange();
            }
        }
    }

    public int getSelectedIndex() {
        return selectedIndex;
    }

    public String getSelectedOption() {
        return selectedIndex >= 0 && selectedIndex < options.length ? options[selectedIndex] : null;
    }

    public void setOnChange(Consumer<Integer> onChange) {
        this.onChange = onChange;
    }

    @Override
    public boolean hasFocus() {
        return focused;
    }

    @Override
    public void setFocus(boolean focused) {
        this.focused = focused;
        if (!focused) {
            expanded = false;
        }
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

        // Draw main dropdown box
        boolean hovered = contains(mouseX, mouseY);
        int bgColor = hovered || focused ? hoverColor : backgroundColor;
        renderer.drawRect(x, y, width, height, bgColor);

        // Draw border
        renderer.drawRect(x, y, width, 1, borderColor);
        renderer.drawRect(x, y + height - 1, width, 1, borderColor);
        renderer.drawRect(x, y, 1, height, borderColor);
        renderer.drawRect(x + width - 1, y, 1, height, borderColor);

        // Draw selected text or placeholder
        String displayText = getDisplayText();
        float textX = x + 8.0f;
        float textY = y + (height - renderer.getLineHeight()) / 2.0f + renderer.getBaselineOffset();
        renderer.drawText(displayText, textX, textY, textColor);

        // Draw dropdown arrow
        float arrowSize = height * 0.4f;
        float arrowX = x + width - arrowSize - 4.0f;
        float arrowY = y + (height - arrowSize) / 2.0f;
        int arrowColor = hovered || focused ? Color.WHITE : Color.rgba(200, 200, 200, 255);
        renderer.drawRect(arrowX, arrowY, arrowSize, arrowSize, arrowColor);

        // Draw expanded options if expanded
        if (expanded) {
            renderOptions(renderer, mouseX, mouseY);
        }
    }

    private void renderOptions(UIRenderer renderer, float mouseX, float mouseY) {
        float x = getX();
        float y = getY();
        float width = getWidth();
        float itemHeight = getHeight();

        // Push scissor for option clipping
        ScissorStack.push((int)x, (int)y, (int)width, (int)(options.length * itemHeight));

        try {
            for (int i = 0; i < options.length; i++) {
                float optionY = y + itemHeight + i * itemHeight;
                boolean optionHovered = mouseX >= x && mouseX <= x + width &&
                                      mouseY >= optionY && mouseY <= optionY + itemHeight;

                // Draw option background
                int optionBgColor = optionHovered ? hoverColor : backgroundColor;
                renderer.drawRect(x, optionY, width, itemHeight, optionBgColor);

                // Draw option text
                String optionText = options[i];
                float textHeight = renderer.getLineHeight();
                float textX = x + 8.0f;
                float textY = optionY + (itemHeight - textHeight) / 2.0f + renderer.getBaselineOffset();
                renderer.drawText(optionText, textX, textY, textColor);

                // Draw selection indicator
                if (i == selectedIndex) {
                    renderer.drawRect(x, optionY, 4, itemHeight, Color.rgba(0, 150, 255, 255));
                }
            }
        } finally {
            ScissorStack.pop();
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
                expanded = !expanded;
                focused = true;
                return true;
            } else if (expanded) {
                // Check if clicking on an option
                if (handleOptionClick(mouseX, mouseY)) {
                    return true;
                }
            }

            // Click outside - close dropdown
            if (focused) {
                expanded = false;
                focused = false;
                return true;
            }
        }

        // Handle mouse release
        if (!input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT) && expanded) {
            handleOptionClick(mouseX, mouseY);
        }

        // Update focus state
        if (hovered && !focused) {
            focused = true;
        } else if (!hovered && !expanded) {
            focused = false;
        }

        return false;
    }

    private boolean handleOptionClick(float mouseX, float mouseY) {
        if (!expanded) {
            return false;
        }

        float x = getX();
        float y = getY();
        float width = getWidth();
        float itemHeight = getHeight();

        for (int i = 0; i < options.length; i++) {
            float optionY = y + itemHeight + i * itemHeight;
            if (mouseX >= x && mouseX <= x + width &&
                mouseY >= optionY && mouseY <= optionY + itemHeight) {
                setSelectedIndex(i);
                expanded = false;
                return true;
            }
        }

        return false;
    }

    private String getDisplayText() {
        if (selectedIndex >= 0 && selectedIndex < options.length) {
            return options[selectedIndex];
        }
        return options.length > 0 ? "Select..." : "No options";
    }

    private void notifyChange() {
        if (onChange != null) {
            onChange.accept(selectedIndex);
        }
    }
}

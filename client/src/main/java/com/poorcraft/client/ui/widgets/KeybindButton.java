package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.common.Constants;

import java.util.function.Consumer;

import static org.lwjgl.glfw.GLFW.*;

/**
 * Keybind button widget for capturing and displaying key bindings.
 */
public class KeybindButton extends Widget implements Panel.FocusableWidget {
    private String keyName;
    private String displayText;
    private Consumer<String> onChange;
    private boolean listening;
    private boolean focused;
    private int normalColor;
    private int hoverColor;
    private int listeningColor;
    private int textColor;

    public KeybindButton(float x, float y, float width, float height, String label, String initialKey) {
        super(x, y, width, height);
        this.keyName = initialKey != null ? initialKey : "None";
        this.displayText = label != null ? label : "Key";
        this.listening = false;
        this.focused = false;
        this.normalColor = Color.rgba(60, 60, 60, 220);
        this.hoverColor = Color.rgba(90, 90, 90, 240);
        this.listeningColor = Color.rgba(100, 60, 0, 240);
        this.textColor = Color.WHITE;
    }

    public void setKey(String key) {
        if (key != null && !key.equals(this.keyName)) {
            this.keyName = key;
            notifyChange();
        }
    }

    public String getKey() {
        return keyName;
    }

    public void setOnChange(Consumer<String> onChange) {
        this.onChange = onChange;
    }

    @Override
    public boolean hasFocus() {
        return focused || listening;
    }

    @Override
    public void setFocus(boolean focused) {
        this.focused = focused;
        if (!focused) {
            listening = false;
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

        // Choose background color
        boolean hovered = contains(mouseX, mouseY);
        int bgColor;
        if (listening) {
            bgColor = listeningColor;
        } else if (hovered || focused) {
            bgColor = hoverColor;
        } else {
            bgColor = normalColor;
        }

        // Draw background
        renderer.drawRect(x, y, width, height, bgColor);

        // Draw border
        renderer.drawRect(x, y, width, 1, Color.BLACK);
        renderer.drawRect(x, y + height - 1, width, 1, Color.BLACK);
        renderer.drawRect(x, y, 1, height, Color.BLACK);
        renderer.drawRect(x + width - 1, y, 1, height, Color.BLACK);

        // Draw label text
        String labelText = displayText + ": ";
        float labelWidth = renderer.measureText(labelText);
        float labelHeight = renderer.getLineHeight();
        float labelX = x + 8.0f;
        float labelY = y + (height - labelHeight) / 2.0f + renderer.getBaselineOffset();
        renderer.drawText(labelText, labelX, labelY, textColor);

        // Draw key text
        String keyText = listening ? "Press key..." : formatKeyName(keyName);
        float keyWidth = renderer.measureText(keyText);
        float keyX = x + width - keyWidth - 8.0f;
        renderer.drawText(keyText, keyX, labelY, textColor);
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        boolean hovered = contains(mouseX, mouseY);

        // Handle mouse click to start listening
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (hovered) {
                listening = true;
                focused = true;
                return true;
            } else if (focused || listening) {
                listening = false;
                focused = false;
                return true;
            }
        }

        // Handle key capture while listening
        if (listening) {
            return handleKeyCapture(input);
        }

        // Update focus state
        if (hovered && !focused) {
            focused = true;
        } else if (!hovered) {
            focused = false;
        }

        return false;
    }

    private boolean handleKeyCapture(InputManager input) {
        // Check for any key press
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++) {
            if (input.keyPressed(key)) {
                String keyName = getKeyName(key);
                if (keyName != null) {
                    setKey(keyName);
                    listening = false;
                    return true;
                }
            }
        }

        // Check mouse buttons
        if (input.mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
            setKey("Mouse Left");
            listening = false;
            return true;
        }
        if (input.mousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            setKey("Mouse Right");
            listening = false;
            return true;
        }
        if (input.mousePressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
            setKey("Mouse Middle");
            listening = false;
            return true;
        }

        // Handle escape to cancel
        if (input.keyPressed(GLFW_KEY_ESCAPE)) {
            listening = false;
            return true;
        }

        return false;
    }

    private String getKeyName(int keyCode) {
        return switch (keyCode) {
            case GLFW_KEY_SPACE -> "Space";
            case GLFW_KEY_APOSTROPHE -> "'";
            case GLFW_KEY_COMMA -> ",";
            case GLFW_KEY_MINUS -> "-";
            case GLFW_KEY_PERIOD -> ".";
            case GLFW_KEY_SLASH -> "/";
            case GLFW_KEY_0 -> "0";
            case GLFW_KEY_1 -> "1";
            case GLFW_KEY_2 -> "2";
            case GLFW_KEY_3 -> "3";
            case GLFW_KEY_4 -> "4";
            case GLFW_KEY_5 -> "5";
            case GLFW_KEY_6 -> "6";
            case GLFW_KEY_7 -> "7";
            case GLFW_KEY_8 -> "8";
            case GLFW_KEY_9 -> "9";
            case GLFW_KEY_SEMICOLON -> ";";
            case GLFW_KEY_EQUAL -> "=";
            case GLFW_KEY_A -> "A";
            case GLFW_KEY_B -> "B";
            case GLFW_KEY_C -> "C";
            case GLFW_KEY_D -> "D";
            case GLFW_KEY_E -> "E";
            case GLFW_KEY_F -> "F";
            case GLFW_KEY_G -> "G";
            case GLFW_KEY_H -> "H";
            case GLFW_KEY_I -> "I";
            case GLFW_KEY_J -> "J";
            case GLFW_KEY_K -> "K";
            case GLFW_KEY_L -> "L";
            case GLFW_KEY_M -> "M";
            case GLFW_KEY_N -> "N";
            case GLFW_KEY_O -> "O";
            case GLFW_KEY_P -> "P";
            case GLFW_KEY_Q -> "Q";
            case GLFW_KEY_R -> "R";
            case GLFW_KEY_S -> "S";
            case GLFW_KEY_T -> "T";
            case GLFW_KEY_U -> "U";
            case GLFW_KEY_V -> "V";
            case GLFW_KEY_W -> "W";
            case GLFW_KEY_X -> "X";
            case GLFW_KEY_Y -> "Y";
            case GLFW_KEY_Z -> "Z";
            case GLFW_KEY_LEFT_BRACKET -> "[";
            case GLFW_KEY_BACKSLASH -> "\\";
            case GLFW_KEY_RIGHT_BRACKET -> "]";
            case GLFW_KEY_GRAVE_ACCENT -> "`";
            case GLFW_KEY_ESCAPE -> "Escape";
            case GLFW_KEY_ENTER -> "Enter";
            case GLFW_KEY_TAB -> "Tab";
            case GLFW_KEY_BACKSPACE -> "Backspace";
            case GLFW_KEY_INSERT -> "Insert";
            case GLFW_KEY_DELETE -> "Delete";
            case GLFW_KEY_RIGHT -> "Right";
            case GLFW_KEY_LEFT -> "Left";
            case GLFW_KEY_DOWN -> "Down";
            case GLFW_KEY_UP -> "Up";
            case GLFW_KEY_PAGE_UP -> "Page Up";
            case GLFW_KEY_PAGE_DOWN -> "Page Down";
            case GLFW_KEY_HOME -> "Home";
            case GLFW_KEY_END -> "End";
            case GLFW_KEY_CAPS_LOCK -> "Caps Lock";
            case GLFW_KEY_SCROLL_LOCK -> "Scroll Lock";
            case GLFW_KEY_NUM_LOCK -> "Num Lock";
            case GLFW_KEY_PRINT_SCREEN -> "Print Screen";
            case GLFW_KEY_PAUSE -> "Pause";
            case GLFW_KEY_F1 -> "F1";
            case GLFW_KEY_F2 -> "F2";
            case GLFW_KEY_F3 -> "F3";
            case GLFW_KEY_F4 -> "F4";
            case GLFW_KEY_F5 -> "F5";
            case GLFW_KEY_F6 -> "F6";
            case GLFW_KEY_F7 -> "F7";
            case GLFW_KEY_F8 -> "F8";
            case GLFW_KEY_F9 -> "F9";
            case GLFW_KEY_F10 -> "F10";
            case GLFW_KEY_F11 -> "F11";
            case GLFW_KEY_F12 -> "F12";
            case GLFW_KEY_F13 -> "F13";
            case GLFW_KEY_F14 -> "F14";
            case GLFW_KEY_F15 -> "F15";
            case GLFW_KEY_F16 -> "F16";
            case GLFW_KEY_F17 -> "F17";
            case GLFW_KEY_F18 -> "F18";
            case GLFW_KEY_F19 -> "F19";
            case GLFW_KEY_F20 -> "F20";
            case GLFW_KEY_F21 -> "F21";
            case GLFW_KEY_F22 -> "F22";
            case GLFW_KEY_F23 -> "F23";
            case GLFW_KEY_F24 -> "F24";
            case GLFW_KEY_F25 -> "F25";
            case GLFW_KEY_KP_0 -> "Numpad 0";
            case GLFW_KEY_KP_1 -> "Numpad 1";
            case GLFW_KEY_KP_2 -> "Numpad 2";
            case GLFW_KEY_KP_3 -> "Numpad 3";
            case GLFW_KEY_KP_4 -> "Numpad 4";
            case GLFW_KEY_KP_5 -> "Numpad 5";
            case GLFW_KEY_KP_6 -> "Numpad 6";
            case GLFW_KEY_KP_7 -> "Numpad 7";
            case GLFW_KEY_KP_8 -> "Numpad 8";
            case GLFW_KEY_KP_9 -> "Numpad 9";
            case GLFW_KEY_KP_DECIMAL -> "Numpad .";
            case GLFW_KEY_KP_DIVIDE -> "Numpad /";
            case GLFW_KEY_KP_MULTIPLY -> "Numpad *";
            case GLFW_KEY_KP_SUBTRACT -> "Numpad -";
            case GLFW_KEY_KP_ADD -> "Numpad +";
            case GLFW_KEY_KP_ENTER -> "Numpad Enter";
            case GLFW_KEY_KP_EQUAL -> "Numpad =";
            case GLFW_KEY_LEFT_SHIFT -> "Left Shift";
            case GLFW_KEY_LEFT_CONTROL -> "Left Ctrl";
            case GLFW_KEY_LEFT_ALT -> "Left Alt";
            case GLFW_KEY_LEFT_SUPER -> "Left Super";
            case GLFW_KEY_RIGHT_SHIFT -> "Right Shift";
            case GLFW_KEY_RIGHT_CONTROL -> "Right Ctrl";
            case GLFW_KEY_RIGHT_ALT -> "Right Alt";
            case GLFW_KEY_RIGHT_SUPER -> "Right Super";
            case GLFW_KEY_MENU -> "Menu";
            default -> null;
        };
    }

    private String formatKeyName(String keyName) {
        if (keyName == null || keyName.isEmpty()) {
            return "None";
        }
        return keyName;
    }

    private void notifyChange() {
        if (onChange != null) {
            onChange.accept(keyName);
        }
    }
}

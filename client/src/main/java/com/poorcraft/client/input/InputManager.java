package com.poorcraft.client.input;

import com.poorcraft.common.Constants;
import com.poorcraft.common.config.Configuration;
import org.lwjgl.glfw.*;
import org.slf4j.Logger;

import java.util.BitSet;

import static org.lwjgl.glfw.GLFW.*;

/**
 * Manages keyboard, mouse, and gamepad input via GLFW callbacks.
 */
public class InputManager implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(InputManager.class);
    
    private final long window;
    
    // Keyboard state
    private final boolean[] keyHeld;
    private final BitSet keyPressed;
    private final BitSet keyReleased;
    
    // Mouse button state
    private final boolean[] mouseHeld;
    private final BitSet mousePressed;
    private final BitSet mouseReleased;
    
    // Mouse position and movement
    private double mouseX;
    private double mouseY;
    private double deltaMouseX;
    private double deltaMouseY;
    private double lastMouseX;
    private double lastMouseY;
    
    // Mouse scroll
    private double scrollX;
    private double scrollY;
    
    // Text input
    private final StringBuilder textBuffer;
    
    // Gamepad state
    private final GamepadState[] gamepads;
    
    // Callbacks (keep references to prevent GC)
    private GLFWKeyCallback keyCallback;
    private GLFWCharCallback charCallback;
    private GLFWCursorPosCallback cursorPosCallback;
    private GLFWMouseButtonCallback mouseButtonCallback;
    private GLFWScrollCallback scrollCallback;
    private GLFWJoystickCallback joystickCallback;
    
    /**
     * Creates a new InputManager.
     * 
     * @param window The GLFW window handle
     * @param config Configuration for input settings
     */
    public InputManager(long window, Configuration config) {
        this.window = window;
        
        // Initialize keyboard state
        this.keyHeld = new boolean[GLFW_KEY_LAST + 1];
        this.keyPressed = new BitSet(GLFW_KEY_LAST + 1);
        this.keyReleased = new BitSet(GLFW_KEY_LAST + 1);
        
        // Initialize mouse state
        this.mouseHeld = new boolean[GLFW_MOUSE_BUTTON_LAST + 1];
        this.mousePressed = new BitSet(GLFW_MOUSE_BUTTON_LAST + 1);
        this.mouseReleased = new BitSet(GLFW_MOUSE_BUTTON_LAST + 1);
        
        // Initialize mouse position
        double[] xpos = new double[1];
        double[] ypos = new double[1];
        glfwGetCursorPos(window, xpos, ypos);
        this.mouseX = xpos[0];
        this.mouseY = ypos[0];
        this.lastMouseX = mouseX;
        this.lastMouseY = mouseY;
        this.deltaMouseX = 0.0;
        this.deltaMouseY = 0.0;
        
        // Initialize scroll
        this.scrollX = 0.0;
        this.scrollY = 0.0;
        
        // Initialize text buffer
        this.textBuffer = new StringBuilder();
        
        // Initialize gamepads
        this.gamepads = new GamepadState[Constants.Input.MAX_GAMEPADS];
        for (int i = 0; i < Constants.Input.MAX_GAMEPADS; i++) {
            gamepads[i] = new GamepadState();
        }
        
        // Register callbacks
        registerCallbacks();
        
        // Enable raw mouse motion if supported and configured
        if (config.getBoolean("input.rawMouseMotion", true)) {
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                LOGGER.info("Raw mouse motion enabled");
            }
        }
        
        LOGGER.info("Input manager initialized");
    }
    
    /**
     * Registers GLFW callbacks.
     */
    private void registerCallbacks() {
        // Keyboard callback
        keyCallback = new GLFWKeyCallback() {
            @Override
            public void invoke(long window, int key, int scancode, int action, int mods) {
                if (key < 0 || key > GLFW_KEY_LAST) return;
                
                if (action == GLFW_PRESS) {
                    keyHeld[key] = true;
                    keyPressed.set(key);
                } else if (action == GLFW_RELEASE) {
                    keyHeld[key] = false;
                    keyReleased.set(key);
                }
                // Ignore GLFW_REPEAT for edge detection
            }
        };
        glfwSetKeyCallback(window, keyCallback);
        
        // Character callback for text input
        charCallback = new GLFWCharCallback() {
            @Override
            public void invoke(long window, int codepoint) {
                textBuffer.appendCodePoint(codepoint);
            }
        };
        glfwSetCharCallback(window, charCallback);
        
        // Cursor position callback
        cursorPosCallback = new GLFWCursorPosCallback() {
            @Override
            public void invoke(long window, double xpos, double ypos) {
                mouseX = xpos;
                mouseY = ypos;
            }
        };
        glfwSetCursorPosCallback(window, cursorPosCallback);
        
        // Mouse button callback
        mouseButtonCallback = new GLFWMouseButtonCallback() {
            @Override
            public void invoke(long window, int button, int action, int mods) {
                if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
                
                if (action == GLFW_PRESS) {
                    mouseHeld[button] = true;
                    mousePressed.set(button);
                } else if (action == GLFW_RELEASE) {
                    mouseHeld[button] = false;
                    mouseReleased.set(button);
                }
            }
        };
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        
        // Scroll callback
        scrollCallback = new GLFWScrollCallback() {
            @Override
            public void invoke(long window, double xoffset, double yoffset) {
                scrollX = xoffset;
                scrollY = yoffset;
            }
        };
        glfwSetScrollCallback(window, scrollCallback);
        
        // Joystick callback for hot-plug
        joystickCallback = new GLFWJoystickCallback() {
            @Override
            public void invoke(int jid, int event) {
                if (jid < 0 || jid >= Constants.Input.MAX_GAMEPADS) return;
                
                if (event == GLFW_CONNECTED) {
                    gamepads[jid].connected = true;
                    LOGGER.info("Gamepad {} connected", jid);
                } else if (event == GLFW_DISCONNECTED) {
                    gamepads[jid].connected = false;
                    LOGGER.info("Gamepad {} disconnected", jid);
                }
            }
        };
        glfwSetJoystickCallback(joystickCallback);
    }
    
    /**
     * Called at the start of each frame to update input state.
     * Should be called after glfwPollEvents().
     */
    public void newFrame() {
        // Clear per-frame edge states
        keyPressed.clear();
        keyReleased.clear();
        mousePressed.clear();
        mouseReleased.clear();
        
        // Calculate mouse delta
        deltaMouseX = mouseX - lastMouseX;
        deltaMouseY = mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        
        // Reset scroll
        scrollX = 0.0;
        scrollY = 0.0;
        
        // Clear text buffer
        textBuffer.setLength(0);
        
        // Update gamepad states
        updateGamepads();
    }
    
    /**
     * Updates gamepad states.
     */
    private void updateGamepads() {
        for (int jid = 0; jid < Constants.Input.MAX_GAMEPADS; jid++) {
            GamepadState state = gamepads[jid];
            
            if (!glfwJoystickPresent(jid)) {
                state.connected = false;
                continue;
            }
            
            state.connected = true;
            
            // Get gamepad state
            GLFWGamepadState glfwState = GLFWGamepadState.create();
            if (glfwGetGamepadState(jid, glfwState)) {
                // Update buttons
                for (int i = 0; i < 15; i++) {
                    boolean pressed = glfwState.buttons(i) == GLFW_PRESS;
                    state.buttonsPressed[i] = pressed && !state.buttons[i];
                    state.buttonsReleased[i] = !pressed && state.buttons[i];
                    state.buttons[i] = pressed;
                }
                
                // Update axes with deadzone
                for (int i = 0; i < 6; i++) {
                    float value = glfwState.axes(i);
                    state.axes[i] = applyDeadzone(value, Constants.Input.DEFAULT_DEADZONE);
                }
            }
        }
    }
    
    /**
     * Applies radial deadzone to an axis value.
     */
    private float applyDeadzone(float value, float deadzone) {
        if (Math.abs(value) < deadzone) {
            return 0.0f;
        }
        // Rescale to 0-1 range outside deadzone
        return (Math.abs(value) - deadzone) / (1.0f - deadzone) * Math.signum(value);
    }
    
    // Keyboard queries
    
    public boolean keyPressed(int key) {
        return key >= 0 && key <= GLFW_KEY_LAST && keyPressed.get(key);
    }
    
    public boolean keyReleased(int key) {
        return key >= 0 && key <= GLFW_KEY_LAST && keyReleased.get(key);
    }
    
    public boolean keyDown(int key) {
        return key >= 0 && key <= GLFW_KEY_LAST && keyHeld[key];
    }
    
    // Mouse queries
    
    public boolean mousePressed(int button) {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && mousePressed.get(button);
    }
    
    public boolean mouseReleased(int button) {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && mouseReleased.get(button);
    }
    
    public boolean mouseDown(int button) {
        return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && mouseHeld[button];
    }
    
    public double getMouseX() { return mouseX; }
    public double getMouseY() { return mouseY; }
    public double getDeltaMouseX() { return deltaMouseX; }
    public double getDeltaMouseY() { return deltaMouseY; }
    
    public double getScrollX() { return scrollX; }
    public double getScrollY() { return scrollY; }
    
    public String getTextInput() {
        return textBuffer.toString();
    }
    
    // Gamepad queries
    
    public GamepadState getGamepad(int jid) {
        if (jid >= 0 && jid < Constants.Input.MAX_GAMEPADS) {
            return gamepads[jid];
        }
        return null;
    }
    
    // Utility methods
    
    public void setCursorMode(int mode) {
        glfwSetInputMode(window, GLFW_CURSOR, mode);
    }
    
    public void enableRawMouseMotion(boolean enable) {
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, enable ? GLFW_TRUE : GLFW_FALSE);
        }
    }
    
    @Override
    public void close() {
        // Free callbacks
        if (keyCallback != null) {
            keyCallback.free();
            keyCallback = null;
        }
        if (charCallback != null) {
            charCallback.free();
            charCallback = null;
        }
        if (cursorPosCallback != null) {
            cursorPosCallback.free();
            cursorPosCallback = null;
        }
        if (mouseButtonCallback != null) {
            mouseButtonCallback.free();
            mouseButtonCallback = null;
        }
        if (scrollCallback != null) {
            scrollCallback.free();
            scrollCallback = null;
        }
        if (joystickCallback != null) {
            joystickCallback.free();
            joystickCallback = null;
        }
        
        LOGGER.info("Input manager closed");
    }
    
    /**
     * Gamepad state container.
     */
    public static class GamepadState {
        public boolean connected;
        public boolean[] buttons = new boolean[15];
        public boolean[] buttonsPressed = new boolean[15];
        public boolean[] buttonsReleased = new boolean[15];
        public float[] axes = new float[6];
    }
}

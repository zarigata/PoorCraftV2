package com.poorcraft.client.ui;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.widgets.Panel;
import com.poorcraft.common.config.Configuration;

import static org.lwjgl.glfw.GLFW.GLFW_CURSOR_DISABLED;
import static org.lwjgl.glfw.GLFW.GLFW_CURSOR_NORMAL;

/**
 * Base class for all UI screens in the retained-mode UI system.
 */
public abstract class UIScreen {
    protected final UIManager uiManager;
    protected final Configuration config;
    protected Panel rootWidget;
    private boolean built;
    private float mouseX, mouseY;

    protected UIScreen(UIManager uiManager, Configuration config) {
        this.uiManager = uiManager;
        this.config = config;
        this.built = false;
    }

    /**
     * Build the UI widget tree. Called once when the screen is first shown.
     */
    protected abstract void buildUI();

    /**
     * Called when the screen becomes active.
     */
    public void onShow() {
        if (!built) {
            buildUI();
            built = true;
        }

        if (rootWidget != null) {
            rootWidget.updateLayout(getScreenWidth(), getScreenHeight());
        }

        updateCursorMode();
    }

    /**
     * Called when the screen becomes inactive.
     */
    public void onHide() {
        // Override in subclasses if needed
    }

    /**
     * Update cursor mode based on screen requirements.
     */
    protected void updateCursorMode() {
        if (uiManager != null) {
            uiManager.setCursorMode(getPreferredCursorMode());
        }
    }

    protected int getPreferredCursorMode() {
        return GLFW_CURSOR_NORMAL;
    }

    /**
     * Update the screen and its widgets.
     */
    public void update(float dt) {
        if (rootWidget != null) {
            updateWidgetTree(rootWidget, dt);
        }
    }

    /**
     * Render the screen and its widgets.
     */
    public void render(UIRenderer renderer) {
        if (rootWidget != null) {
            rootWidget.render(renderer, mouseX, mouseY);
        }
    }

    /**
     * Handle input for the screen and its widgets.
     * @return true if the screen consumed the input, false otherwise
     */
    public boolean handleInput(InputManager input) {
        // Update mouse position
        double[] mouseX = new double[1];
        double[] mouseY = new double[1];
        input.getMousePosition(mouseX, mouseY);
        this.mouseX = (float) mouseX[0];
        this.mouseY = (float) mouseY[0];

        if (rootWidget != null && rootWidget.handleInput(input, this.mouseX, this.mouseY)) {
            return true;
        }

        return shouldConsumeInput();
    }

    protected boolean shouldConsumeInput() {
        return false;
    }

    /**
     * Handle window resize.
     */
    public void resize(int width, int height) {
        if (rootWidget != null) {
            rootWidget.updateLayout(width, height);
        }
    }

    /**
     * Clean up the screen resources.
     */
    public void cleanup() {
        if (rootWidget != null) {
            cleanupWidgetTree(rootWidget);
        }
        rootWidget = null;
        built = false;
    }

    /**
     * Get the root widget of this screen.
     */
    protected Panel getRootWidget() {
        return rootWidget;
    }

    /**
     * Set the root widget of this screen.
     */
    protected void setRootWidget(Panel rootWidget) {
        if (this.rootWidget != null) {
            cleanupWidgetTree(this.rootWidget);
        }
        this.rootWidget = rootWidget;
        if (rootWidget != null && built) {
            rootWidget.updateLayout(getScreenWidth(), getScreenHeight());
        }
    }

    /**
     * Get the current screen width.
     */
    protected int getScreenWidth() {
        return uiManager != null ? uiManager.getViewportWidth() : 800;
    }

    /**
     * Get the current screen height.
     */
    protected int getScreenHeight() {
        return uiManager != null ? uiManager.getViewportHeight() : 600;
    }

    /**
     * Recursively update a widget tree.
     */
    private void updateWidgetTree(Widget widget, float dt) {
        // Update the widget if it has an update method
        if (widget instanceof UpdatableWidget) {
            ((UpdatableWidget) widget).update(dt);
        }

        // Update children
        for (Widget child : widget.getChildren()) {
            updateWidgetTree(child, dt);
        }
    }

    /**
     * Recursively clean up a widget tree.
     */
    private void cleanupWidgetTree(Widget widget) {
        // Cleanup the widget if it has a cleanup method
        if (widget instanceof CleanupableWidget) {
            ((CleanupableWidget) widget).cleanup();
        }

        // Cleanup children
        for (Widget child : widget.getChildren()) {
            cleanupWidgetTree(child);
        }
    }

    /**
     * Interface for widgets that need updates.
     */
    public interface UpdatableWidget {
        void update(float dt);
    }

    /**
     * Interface for widgets that need cleanup.
     */
    public interface CleanupableWidget {
        void cleanup();
    }
}

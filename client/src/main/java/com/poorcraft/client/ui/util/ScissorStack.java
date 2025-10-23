package com.poorcraft.client.ui.util;

import org.lwjgl.opengl.GL11;

import java.util.Stack;

/**
 * Utility class for managing OpenGL scissor test stack for UI clipping.
 */
public class ScissorStack {
    private static final Stack<ScissorArea> stack = new Stack<>();

    /**
     * Represents a scissor area for clipping.
     */
    public static class ScissorArea {
        public final int x, y, width, height;

        public ScissorArea(int x, int y, int width, int height) {
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }
    }

    /**
     * Push a new scissor area onto the stack.
     */
    public static void push(int x, int y, int width, int height) {
        ScissorArea parent = stack.isEmpty() ? null : stack.peek();

        if (parent != null) {
            // Intersect with parent area
            int newX = Math.max(parent.x, x);
            int newY = Math.max(parent.y, y);
            int newWidth = Math.min(parent.x + parent.width, x + width) - newX;
            int newHeight = Math.min(parent.y + parent.height, y + height) - newY;

            // Ensure valid area
            if (newWidth > 0 && newHeight > 0) {
                stack.push(new ScissorArea(newX, newY, newWidth, newHeight));
            } else {
                // Invalid intersection, push empty area
                stack.push(new ScissorArea(0, 0, 0, 0));
            }
        } else {
            stack.push(new ScissorArea(x, y, width, height));
        }

        applyCurrentScissor();
    }

    /**
     * Pop the current scissor area from the stack.
     */
    public static void pop() {
        if (!stack.isEmpty()) {
            stack.pop();
        }
        applyCurrentScissor();
    }

    /**
     * Apply the current scissor area to OpenGL.
     */
    private static void applyCurrentScissor() {
        if (stack.isEmpty()) {
            GL11.glDisable(GL11.GL_SCISSOR_TEST);
        } else {
            ScissorArea current = stack.peek();
            GL11.glEnable(GL11.GL_SCISSOR_TEST);
            GL11.glScissor(current.x, current.y, current.width, current.height);
        }
    }

    /**
     * Get the current scissor area without popping.
     */
    public static ScissorArea peek() {
        return stack.isEmpty() ? null : stack.peek();
    }

    /**
     * Clear the entire scissor stack.
     */
    public static void clear() {
        while (!stack.isEmpty()) {
            stack.pop();
        }
        GL11.glDisable(GL11.GL_SCISSOR_TEST);
    }

    /**
     * Check if scissor test is currently enabled.
     */
    public static boolean isEnabled() {
        return !stack.isEmpty();
    }
}

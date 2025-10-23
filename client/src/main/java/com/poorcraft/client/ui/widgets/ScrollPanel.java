package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.client.ui.util.ScissorStack;
import com.poorcraft.common.Constants;

import java.util.List;

/**
 * Scrollable panel widget that clips content and provides scrolling.
 */
public class ScrollPanel extends Widget implements Panel.FocusableWidget {
    private float scrollY;
    private float maxScrollY;
    private float contentHeight;
    private boolean showScrollbar;
    private boolean draggingScrollbar;
    private boolean focused;
    private int backgroundColor;
    private int scrollbarColor;
    private int scrollbarHoverColor;

    public ScrollPanel(float x, float y, float width, float height) {
        super(x, y, width, height);
        this.scrollY = 0.0f;
        this.maxScrollY = 0.0f;
        this.contentHeight = 0.0f;
        this.showScrollbar = true;
        this.draggingScrollbar = false;
        this.focused = false;
        this.backgroundColor = Color.TRANSPARENT; // Transparent by default
        this.scrollbarColor = Color.rgba(150, 150, 150, 200);
        this.scrollbarHoverColor = Color.rgba(200, 200, 200, 255);
    }

    public void setContentHeight(float contentHeight) {
        this.contentHeight = contentHeight;
        updateMaxScroll();
    }

    public void scrollTo(float y) {
        this.scrollY = Math.max(0.0f, Math.min(maxScrollY, y));
    }

    public void scrollBy(float deltaY) {
        scrollTo(scrollY - deltaY);
    }

    public float getScrollY() {
        return scrollY;
    }

    public void setShowScrollbar(boolean showScrollbar) {
        this.showScrollbar = showScrollbar;
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

        // Push scissor area for content clipping
        ScissorStack.push((int)x, (int)y, (int)width, (int)height);

        try {
            // Draw background if not transparent
            if (backgroundColor != Color.TRANSPARENT) {
                renderer.drawRect(x, y, width, height, backgroundColor);
            }

            // Render children with scroll offset
            for (Widget child : getChildren()) {
                if (child.isVisible()) {
                    // Apply scroll offset to child position
                    float childX = child.getLocalX() + x;
                    float childY = child.getLocalY() + y - scrollY;

                    // Only render if child is visible in scroll area
                    if (childY + child.getHeight() >= y && childY <= y + height) {
                        // Temporarily modify child's absolute position for rendering
                        float originalAbsX = child.getX();
                        float originalAbsY = child.getY();
                        child.setAbsolutePosition(childX, childY);

                        child.render(renderer, mouseX, mouseY);

                        // Restore original position
                        child.setAbsolutePosition(originalAbsX, originalAbsY);
                    }
                }
            }
        } finally {
            ScissorStack.pop();
        }

        // Draw scrollbar if needed and enabled
        if (showScrollbar && maxScrollY > 0.0f) {
            drawScrollbar(renderer, mouseX, mouseY);
        }
    }

    private void drawScrollbar(UIRenderer renderer, float mouseX, float mouseY) {
        float x = getX();
        float y = getY();
        float width = getWidth();
        float height = getHeight();

        float scrollbarWidth = 12.0f;
        float scrollbarX = x + width - scrollbarWidth;

        // Draw scrollbar track
        renderer.drawRect(scrollbarX, y, scrollbarWidth, height, scrollbarColor);

        // Calculate scrollbar thumb size and position
        float trackHeight = height;
        float thumbHeight = Math.max(20.0f, trackHeight * (height / contentHeight));
        float thumbY = y + (scrollY / maxScrollY) * (trackHeight - thumbHeight);

        // Draw scrollbar thumb
        boolean thumbHovered = mouseX >= scrollbarX && mouseX <= scrollbarX + scrollbarWidth &&
                              mouseY >= thumbY && mouseY <= thumbY + thumbHeight;
        int thumbColor = thumbHovered ? scrollbarHoverColor : Color.WHITE;
        renderer.drawRect(scrollbarX, thumbY, scrollbarWidth, thumbHeight, thumbColor);
    }

    @Override
    public boolean handleInput(InputManager input, float mouseX, float mouseY) {
        if (!isVisible() || !isEnabled()) {
            return false;
        }

        boolean handled = false;
        float x = getX();
        float y = getY();
        float width = getWidth();
        float height = getHeight();

        // Handle scrollbar dragging
        if (showScrollbar && maxScrollY > 0.0f) {
            handled = handleScrollbarInput(input, mouseX, mouseY);
        }

        // Handle mouse wheel scrolling
        double scrollDelta = input.getScrollY();
        if (scrollDelta != 0.0) {
            scrollBy((float) scrollDelta * 20.0f); // Adjust scroll speed
            handled = true;
        }

        // Handle children input (with scroll offset)
        if (!handled) {
            List<Widget> children = getChildren();
            for (int i = children.size() - 1; i >= 0; i--) {
                Widget child = children.get(i);
                if (!child.isVisible() || !child.isEnabled()) {
                    continue;
                }

                // Check if mouse is over child (with scroll offset)
                float childAbsX = child.getLocalX() + x;
                float childAbsY = child.getLocalY() + y - scrollY;

                if (mouseX >= childAbsX && mouseX <= childAbsX + child.getWidth() &&
                    mouseY >= childAbsY && mouseY <= childAbsY + child.getHeight()) {

                    // Temporarily modify child's absolute position for input handling
                    float originalAbsX = child.getX();
                    float originalAbsY = child.getY();
                    child.setAbsolutePosition(childAbsX, childAbsY);

                    if (child.handleInput(input, mouseX, mouseY)) {
                        handled = true;
                    }

                    // Restore original position
                    child.setAbsolutePosition(originalAbsX, originalAbsY);

                    if (handled) {
                        break;
                    }
                }
            }
        }

        // Update focus state
        boolean mouseOverPanel = contains(mouseX, mouseY);
        if (mouseOverPanel && !focused) {
            focused = true;
        } else if (!mouseOverPanel && !draggingScrollbar) {
            focused = false;
        }

        return handled;
    }

    private boolean handleScrollbarInput(InputManager input, float mouseX, float mouseY) {
        float x = getX();
        float y = getY();
        float width = getWidth();
        float height = getHeight();

        float scrollbarWidth = 12.0f;
        float scrollbarX = x + width - scrollbarWidth;

        // Calculate thumb position and size
        float trackHeight = height;
        float thumbHeight = Math.max(20.0f, trackHeight * (height / contentHeight));
        float thumbY = y + (scrollY / maxScrollY) * (trackHeight - thumbHeight);

        boolean thumbHovered = mouseX >= scrollbarX && mouseX <= scrollbarX + scrollbarWidth &&
                              mouseY >= thumbY && mouseY <= thumbY + thumbHeight;

        // Handle scrollbar thumb dragging
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (thumbHovered) {
                draggingScrollbar = true;
                return true;
            }
        }

        if (draggingScrollbar) {
            if (input.mouseDown(Constants.Input.MOUSE_BUTTON_LEFT)) {
                // Update scroll position based on mouse Y
                float relativeY = mouseY - y;
                float thumbPosition = relativeY / trackHeight;
                thumbPosition = Math.max(0.0f, Math.min(1.0f, thumbPosition));
                scrollTo(thumbPosition * maxScrollY);
                return true;
            } else {
                draggingScrollbar = false;
            }
        }

        // Handle scrollbar track clicks
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            if (mouseX >= scrollbarX && mouseX <= scrollbarX + scrollbarWidth) {
                if (mouseY < thumbY) {
                    // Click above thumb - scroll up
                    scrollBy(height);
                    return true;
                } else if (mouseY > thumbY + thumbHeight) {
                    // Click below thumb - scroll down
                    scrollBy(-height);
                    return true;
                }
            }
        }

        return false;
    }

    private void updateMaxScroll() {
        maxScrollY = Math.max(0.0f, contentHeight - getHeight());
        if (scrollY > maxScrollY) {
            scrollY = maxScrollY;
        }
    }
}

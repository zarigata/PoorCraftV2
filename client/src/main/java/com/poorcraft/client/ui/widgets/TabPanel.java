package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.input.InputManager;
import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;
import com.poorcraft.client.ui.util.ScissorStack;
import com.poorcraft.common.Constants;

import java.util.ArrayList;
import java.util.List;

/**
 * Tab panel widget for organizing content in tabs.
 */
public class TabPanel extends Widget implements Panel.FocusableWidget {
    private final List<Tab> tabs;
    private int activeTabIndex;
    private boolean focused;
    private int tabBarColor;
    private int activeTabColor;
    private int inactiveTabColor;
    private int hoverTabColor;
    private int textColor;

    public TabPanel(float x, float y, float width, float height) {
        super(x, y, width, height);
        this.tabs = new ArrayList<>();
        this.activeTabIndex = -1;
        this.focused = false;
        this.tabBarColor = Color.rgba(80, 80, 80, 200);
        this.activeTabColor = Color.rgba(120, 120, 120, 220);
        this.inactiveTabColor = Color.rgba(60, 60, 60, 200);
        this.hoverTabColor = Color.rgba(100, 100, 100, 220);
        this.textColor = Color.WHITE;
    }

    public void addTab(String title, Widget content) {
        Tab tab = new Tab(title, content);
        tabs.add(tab);

        if (activeTabIndex == -1) {
            activeTabIndex = 0;
        }

        // Position content below tab bar
        if (content != null) {
            content.setPosition(0, getTabBarHeight());
        }
    }

    public void setActiveTab(int index) {
        if (index >= 0 && index < tabs.size() && index != activeTabIndex) {
            activeTabIndex = index;
        }
    }

    public int getActiveTabIndex() {
        return activeTabIndex;
    }

    public String getActiveTabTitle() {
        if (activeTabIndex >= 0 && activeTabIndex < tabs.size()) {
            return tabs.get(activeTabIndex).title;
        }
        return null;
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
        float tabBarHeight = getTabBarHeight();

        // Draw tab bar background
        renderer.drawRect(x, y, width, tabBarHeight, tabBarColor);

        // Draw tabs
        float tabX = x;
        for (int i = 0; i < tabs.size(); i++) {
            Tab tab = tabs.get(i);
            float tabWidth = getTabWidth(i);

            // Choose tab color
            int tabColor;
            if (i == activeTabIndex) {
                tabColor = activeTabColor;
            } else {
                boolean hovered = isTabHovered(i, mouseX, mouseY);
                tabColor = hovered ? hoverTabColor : inactiveTabColor;
            }

            // Draw tab background
            renderer.drawRect(tabX, y, tabWidth, tabBarHeight, tabColor);

            // Draw tab border
            renderer.drawRect(tabX, y, tabWidth, 1, Color.BLACK);
            renderer.drawRect(tabX, y + tabBarHeight - 1, tabWidth, 1, Color.BLACK);
            if (i == 0) {
                renderer.drawRect(tabX, y, 1, tabBarHeight, Color.BLACK);
            }
            if (i == tabs.size() - 1) {
                renderer.drawRect(tabX + tabWidth - 1, y, 1, tabBarHeight, Color.BLACK);
            }

            // Draw tab text
            String tabText = tab.title;
            float textWidth = renderer.measureText(tabText);
            float textHeight = renderer.getLineHeight();
            float textX = tabX + (tabWidth - textWidth) / 2.0f;
            float textY = y + (tabBarHeight - textHeight) / 2.0f + renderer.getBaselineOffset();
            renderer.drawText(tabText, textX, textY, textColor);

            tabX += tabWidth;
        }

        // Draw active tab indicator
        if (activeTabIndex >= 0 && activeTabIndex < tabs.size()) {
            float activeTabX = x;
            for (int i = 0; i < activeTabIndex; i++) {
                activeTabX += getTabWidth(i);
            }
            float activeTabWidth = getTabWidth(activeTabIndex);
            renderer.drawRect(activeTabX, y + tabBarHeight - 2, activeTabWidth, 2, Color.rgba(0, 150, 255, 255));
        }

        // Draw tab content area
        float contentY = y + tabBarHeight;
        float contentHeight = height - tabBarHeight;

        // Push scissor for content clipping
        ScissorStack.push((int)x, (int)contentY, (int)width, (int)contentHeight);

        try {
            // Draw content background
            renderer.drawRect(x, contentY, width, contentHeight, Color.rgba(40, 40, 40, 200));

            // Render active tab content
            if (activeTabIndex >= 0 && activeTabIndex < tabs.size()) {
                Tab activeTab = tabs.get(activeTabIndex);
                if (activeTab.content != null && activeTab.content.isVisible()) {
                    // Temporarily set content position
                    float originalX = activeTab.content.getLocalX();
                    float originalY = activeTab.content.getLocalY();
                    activeTab.content.setPosition(originalX, originalY);

                    activeTab.content.render(renderer, mouseX, mouseY);

                    // Restore original position
                    activeTab.content.setPosition(originalX, originalY);
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

        boolean handled = false;

        // Handle tab clicks
        if (input.mousePressed(Constants.Input.MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < tabs.size(); i++) {
                if (isTabHovered(i, mouseX, mouseY)) {
                    setActiveTab(i);
                    handled = true;
                    break;
                }
            }
        }

        // Handle active tab content input
        if (!handled && activeTabIndex >= 0 && activeTabIndex < tabs.size()) {
            Tab activeTab = tabs.get(activeTabIndex);
            if (activeTab.content != null) {
                // Check if mouse is over content area
                float x = getX();
                float y = getY();
                float width = getWidth();
                float tabBarHeight = getTabBarHeight();
                float contentY = y + tabBarHeight;
                float contentHeight = getHeight() - tabBarHeight;

                if (mouseX >= x && mouseX <= x + width &&
                    mouseY >= contentY && mouseY <= contentY + contentHeight) {

                    if (activeTab.content.handleInput(input, mouseX, mouseY)) {
                        handled = true;
                    }
                }
            }
        }

        // Update focus state
        boolean mouseOverPanel = contains(mouseX, mouseY);
        if (mouseOverPanel && !focused) {
            focused = true;
        } else if (!mouseOverPanel) {
            focused = false;
        }

        return handled;
    }

    private boolean isTabHovered(int tabIndex, float mouseX, float mouseY) {
        if (tabIndex < 0 || tabIndex >= tabs.size()) {
            return false;
        }

        float x = getX();
        float y = getY();
        float tabBarHeight = getTabBarHeight();

        float tabX = x;
        for (int i = 0; i < tabIndex; i++) {
            tabX += getTabWidth(i);
        }
        float tabWidth = getTabWidth(tabIndex);

        return mouseX >= tabX && mouseX <= tabX + tabWidth &&
               mouseY >= y && mouseY <= y + tabBarHeight;
    }

    private float getTabWidth(int tabIndex) {
        if (tabIndex < 0 || tabIndex >= tabs.size()) {
            return 0.0f;
        }

        float totalWidth = getWidth();
        float tabBarHeight = getTabBarHeight();

        // Simple equal-width tabs for now
        return totalWidth / Math.max(1, tabs.size());
    }

    private float getTabBarHeight() {
        return 30.0f; // Fixed height for tab bar
    }

    /**
     * Represents a single tab with title and content.
     */
    private static class Tab {
        final String title;
        final Widget content;

        Tab(String title, Widget content) {
            this.title = title;
            this.content = content;
        }
    }
}

package com.poorcraft.client.ui.layout;

import com.poorcraft.client.ui.Widget;

import java.util.List;

public final class LayoutManager {
    private LayoutManager() {
    }

    public static void calculateAnchoredPosition(Widget widget, int parentWidth, int parentHeight) {
        float anchorX = widget.getAnchor().getAnchorX();
        float anchorY = widget.getAnchor().getAnchorY();
        float x = parentWidth * anchorX - widget.getWidth() * anchorX + widget.getOffsetX();
        float y = parentHeight * anchorY - widget.getHeight() * anchorY + widget.getOffsetY();
        widget.setAbsolutePosition(x, y);
    }

    public static void layoutVertical(List<Widget> widgets, float startY, float spacing) {
        float currentY = startY;
        for (Widget widget : widgets) {
            widget.setAbsoluteY(currentY);
            currentY += widget.getHeight() + spacing;
        }
    }

    public static void layoutHorizontal(List<Widget> widgets, float startX, float spacing) {
        float currentX = startX;
        for (Widget widget : widgets) {
            widget.setAbsoluteX(currentX);
            currentX += widget.getWidth() + spacing;
        }
    }

    public static void layoutGrid(List<Widget> widgets, int columns, float startX, float startY, float spacingX, float spacingY) {
        if (columns <= 0) {
            return;
        }
        float currentX = startX;
        float currentY = startY;
        int column = 0;
        for (Widget widget : widgets) {
            widget.setAbsolutePosition(currentX, currentY);
            column++;
            if (column >= columns) {
                column = 0;
                currentX = startX;
                currentY += widget.getHeight() + spacingY;
            } else {
                currentX += widget.getWidth() + spacingX;
            }
        }
    }

    public static void centerWidget(Widget widget, int parentWidth, int parentHeight) {
        float x = (parentWidth - widget.getWidth()) * 0.5f;
        float y = (parentHeight - widget.getHeight()) * 0.5f;
        widget.setAbsolutePosition(x, y);
    }
}

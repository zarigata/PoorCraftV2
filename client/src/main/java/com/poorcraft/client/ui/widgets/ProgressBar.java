package com.poorcraft.client.ui.widgets;

import com.poorcraft.client.ui.Color;
import com.poorcraft.client.ui.UIRenderer;
import com.poorcraft.client.ui.Widget;

/**
 * Progress bar widget for displaying progress or loading status.
 */
public class ProgressBar extends Widget {
    private float progress; // 0.0 to 1.0
    private float minValue;
    private float maxValue;
    private boolean showText;
    private String textFormat;
    private int backgroundColor;
    private int fillColor;
    private int textColor;

    public ProgressBar(float x, float y, float width, float height, float initialProgress) {
        super(x, y, width, height);
        this.progress = Math.max(0.0f, Math.min(1.0f, initialProgress));
        this.minValue = 0.0f;
        this.maxValue = 1.0f;
        this.showText = false;
        this.textFormat = "%.0f%%";
        this.backgroundColor = Color.rgba(100, 100, 100, 200);
        this.fillColor = Color.rgba(0, 150, 0, 255);
        this.textColor = Color.WHITE;
    }

    public void setProgress(float progress) {
        this.progress = Math.max(0.0f, Math.min(1.0f, progress));
    }

    public void setValue(float value) {
        setProgress((value - minValue) / (maxValue - minValue));
    }

    public void setRange(float minValue, float maxValue) {
        this.minValue = minValue;
        this.maxValue = maxValue;
    }

    public float getProgress() {
        return progress;
    }

    public float getValue() {
        return minValue + progress * (maxValue - minValue);
    }

    public void setShowText(boolean showText) {
        this.showText = showText;
    }

    public void setTextFormat(String format) {
        this.textFormat = format;
    }

    public void setColors(int backgroundColor, int fillColor, int textColor) {
        this.backgroundColor = backgroundColor;
        this.fillColor = fillColor;
        this.textColor = textColor;
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

        // Draw background
        renderer.drawRect(x, y, width, height, backgroundColor);

        // Draw progress fill
        float fillWidth = width * progress;
        if (fillWidth > 0) {
            renderer.drawRect(x, y, fillWidth, height, fillColor);
        }

        // Draw border
        renderer.drawRect(x, y, width, 1, Color.BLACK);
        renderer.drawRect(x, y + height - 1, width, 1, Color.BLACK);
        renderer.drawRect(x, y, 1, height, Color.BLACK);
        renderer.drawRect(x + width - 1, y, 1, height, Color.BLACK);

        // Draw text if enabled
        if (showText) {
            String text = String.format(textFormat, getValue() * 100.0f);
            float textWidth = renderer.measureText(text);
            float textHeight = renderer.getLineHeight();
            float textX = x + (width - textWidth) / 2.0f;
            float textY = y + (height - textHeight) / 2.0f + renderer.getBaselineOffset();
            renderer.drawText(text, textX, textY, textColor);
        }
    }

    @Override
    public boolean handleInput(com.poorcraft.client.input.InputManager input, float mouseX, float mouseY) {
        // Progress bars are typically read-only
        return false;
    }
}

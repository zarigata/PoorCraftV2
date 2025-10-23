package com.poorcraft.client.ui.util;

import com.poorcraft.client.ui.Font;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;

public final class UIUtil {
    private static final DecimalFormat NUMBER_FORMAT = new DecimalFormat("###,###");

    private UIUtil() {
    }

    public static boolean isMouseOver(float mouseX, float mouseY, float x, float y, float width, float height) {
        return mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
    }

    public static float clamp(float value, float min, float max) {
        return Math.max(min, Math.min(max, value));
    }

    public static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    public static float easeInOut(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    public static String formatNumber(int number) {
        return NUMBER_FORMAT.format(number);
    }

    public static String formatTime(long millis) {
        long totalSeconds = millis / 1000L;
        long minutes = totalSeconds / 60L;
        long seconds = totalSeconds % 60L;
        return String.format("%02d:%02d", minutes, seconds);
    }

    public static List<String> wrapText(String text, float maxWidth, Font font) {
        List<String> lines = new ArrayList<>();
        if (text == null || text.isEmpty()) {
            lines.add("");
            return lines;
        }

        String[] words = text.split(" ");
        StringBuilder currentLine = new StringBuilder();
        for (String word : words) {
            String candidate = currentLine.length() == 0 ? word : currentLine + " " + word;
            if (font.getTextWidth(candidate) <= maxWidth) {
                currentLine.setLength(0);
                currentLine.append(candidate);
            } else {
                if (currentLine.length() > 0) {
                    lines.add(currentLine.toString());
                    currentLine.setLength(0);
                }
                if (font.getTextWidth(word) > maxWidth) {
                    lines.add(word);
                } else {
                    currentLine.append(word);
                }
            }
        }
        if (currentLine.length() > 0) {
            lines.add(currentLine.toString());
        }
        return lines;
    }

    public static float scaleUI(float value, float scale) {
        return value * scale;
    }
}

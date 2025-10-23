package com.poorcraft.client.ui;

public final class Color {
    public static final int WHITE = rgba(255, 255, 255, 255);
    public static final int BLACK = rgba(0, 0, 0, 255);
    public static final int RED = rgba(255, 0, 0, 255);
    public static final int GREEN = rgba(0, 255, 0, 255);
    public static final int BLUE = rgba(0, 0, 255, 255);
    public static final int YELLOW = rgba(255, 255, 0, 255);
    public static final int GRAY = rgba(128, 128, 128, 255);
    public static final int LIGHT_GRAY = rgba(192, 192, 192, 255);
    public static final int DARK_GRAY = rgba(64, 64, 64, 255);
    public static final int TRANSPARENT = rgba(0, 0, 0, 0);

    private Color() {
    }

    public static int rgb(int r, int g, int b) {
        return rgba(r, g, b, 255);
    }

    public static int rgba(int r, int g, int b, int a) {
        int color = 0;
        color |= (clamp(r) & 0xFF) << 24;
        color |= (clamp(g) & 0xFF) << 16;
        color |= (clamp(b) & 0xFF) << 8;
        color |= clamp(a) & 0xFF;
        return color;
    }

    public static int fromHex(String hex) {
        String cleaned = hex.startsWith("#") ? hex.substring(1) : hex;
        if (cleaned.length() == 6) {
            cleaned = cleaned + "FF";
        }
        if (cleaned.length() != 8) {
            throw new IllegalArgumentException("Invalid hex color: " + hex);
        }
        int r = Integer.parseInt(cleaned.substring(0, 2), 16);
        int g = Integer.parseInt(cleaned.substring(2, 4), 16);
        int b = Integer.parseInt(cleaned.substring(4, 6), 16);
        int a = Integer.parseInt(cleaned.substring(6, 8), 16);
        return rgba(r, g, b, a);
    }

    public static int getRed(int color) {
        return (color >>> 24) & 0xFF;
    }

    public static int getGreen(int color) {
        return (color >>> 16) & 0xFF;
    }

    public static int getBlue(int color) {
        return (color >>> 8) & 0xFF;
    }

    public static int getAlpha(int color) {
        return color & 0xFF;
    }

    public static int withAlpha(int color, int alpha) {
        return (color & 0xFFFFFF00) | (clamp(alpha) & 0xFF);
    }

    public static float[] toFloatArray(int color) {
        return new float[] {
                getRed(color) / 255.0f,
                getGreen(color) / 255.0f,
                getBlue(color) / 255.0f,
                getAlpha(color) / 255.0f
        };
    }

    private static int clamp(int value) {
        return Math.max(0, Math.min(255, value));
    }
}

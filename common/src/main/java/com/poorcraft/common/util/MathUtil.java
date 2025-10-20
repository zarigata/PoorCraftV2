package com.poorcraft.common.util;

/**
 * Utility class for common mathematical operations.
 */
public final class MathUtil {
    
    private MathUtil() {
        // Prevent instantiation
    }
    
    /**
     * Clamps an integer value between min and max.
     */
    public static int clamp(int value, int min, int max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Clamps a float value between min and max.
     */
    public static float clamp(float value, float min, float max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Clamps a double value between min and max.
     */
    public static double clamp(double value, double min, double max) {
        return Math.max(min, Math.min(max, value));
    }
    
    /**
     * Linear interpolation between a and b.
     */
    public static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    
    /**
     * Linear interpolation between a and b.
     */
    public static double lerp(double a, double b, double t) {
        return a + t * (b - a);
    }
    
    /**
     * Smooth interpolation using smoothstep function.
     */
    public static double smoothstep(double edge0, double edge1, double x) {
        double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    }
    
    /**
     * Floor division that handles negatives correctly.
     */
    public static int floorDiv(int a, int b) {
        return Math.floorDiv(a, b);
    }
    
    /**
     * Floor modulo that handles negatives correctly.
     */
    public static int floorMod(int a, int b) {
        return Math.floorMod(a, b);
    }
    
    /**
     * Fast floor operation for doubles.
     */
    public static int fastFloor(double x) {
        int xi = (int) x;
        return x < xi ? xi - 1 : xi;
    }
    
    /**
     * Checks if a number is a power of two.
     */
    public static boolean isPowerOfTwo(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }
    
    /**
     * Rounds up to the next power of two.
     */
    public static int nextPowerOfTwo(int n) {
        if (n <= 0) return 1;
        if (isPowerOfTwo(n)) return n;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n + 1;
    }
}

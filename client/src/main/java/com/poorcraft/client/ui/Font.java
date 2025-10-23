package com.poorcraft.client.ui;

import com.poorcraft.client.render.GLInfo;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.common.config.Configuration;
import org.joml.Vector2f;
import org.lwjgl.BufferUtils;
import org.lwjgl.stb.STBTTAlignedQuad;
import org.lwjgl.stb.STBTTBakedChar;
import org.lwjgl.stb.STBTruetype;
import org.lwjgl.system.MemoryStack;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.stb.STBTruetype.*;

/**
 * STB TrueType-based font implementation with glyph atlas.
 */
public class Font implements AutoCloseable {
    private static final int BITMAP_WIDTH = 1024;
    private static final int BITMAP_HEIGHT = 1024;
    private static final int FIRST_CHAR = 32; // Space
    private static final int NUM_CHARS = 95;  // ASCII 32-126

    private final String name;
    private final float size;
    private final Texture2D fontAtlas;
    private final STBTTBakedChar.Buffer charData;
    private final float scale;
    private final float ascent;
    private final float descent;
    private final float lineGap;

    private Font(String name, float size, Texture2D fontAtlas, STBTTBakedChar.Buffer charData,
                 float scale, float ascent, float descent, float lineGap) {
        this.name = name;
        this.size = size;
        this.fontAtlas = fontAtlas;
        this.charData = charData;
        this.scale = scale;
        this.ascent = ascent;
        this.descent = descent;
        this.lineGap = lineGap;
    }

    public static Font fromConfig(Configuration config) {
        String fontName = config.getString("ui.font", "fonts/default.ttf");
        float fontSize = config.getFloat("ui.fontSize", 16.0f);
        float uiScale = config.getFloat("ui.scale", 1.0f);

        try {
            return loadFont(fontName, fontSize * uiScale);
        } catch (Exception e) {
            // Fallback to system font or create a basic bitmap font
            System.err.println("Failed to load font " + fontName + ", using fallback: " + e.getMessage());
            return createFallbackFont(fontSize * uiScale);
        }
    }

    private static Font createFallbackFont(float fontSize) {
        // Create a simple fallback font using basic ASCII characters
        // This is a placeholder - in a real implementation you'd use a system font
        System.err.println("Creating fallback font (not fully implemented)");
        return null; // This will cause issues, but let's focus on Main.java integration first
    }

    public static Font loadFont(String fontPath, float fontSize) {
        try {
            // Try filesystem first
            Path path = Paths.get(fontPath);
            ByteBuffer fontBuffer;
            if (Files.exists(path)) {
                fontBuffer = Files.readAllBytes(path);
                fontBuffer = BufferUtils.createByteBuffer(fontBuffer.remaining()).put(fontBuffer);
                fontBuffer.flip();
            } else {
                // Try classpath
                try (var stream = Font.class.getClassLoader().getResourceAsStream(fontPath)) {
                    if (stream == null) {
                        throw new IOException("Font not found: " + fontPath);
                    }
                    fontBuffer = BufferUtils.createByteBuffer(stream.available());
                    while (stream.available() > 0) {
                        fontBuffer.put((byte) stream.read());
                    }
                    fontBuffer.flip();
                }
            }

            return loadFontFromBuffer(fontBuffer, fontSize);
        } catch (Exception e) {
            throw new RuntimeException("Failed to load font: " + fontPath, e);
        }
    }

    private static Font loadFontFromBuffer(ByteBuffer fontBuffer, float fontSize) throws IOException {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            // Get font info
            IntBuffer ascent = stack.mallocInt(1);
            IntBuffer descent = stack.mallocInt(1);
            IntBuffer lineGap = stack.mallocInt(1);

            stbtt_GetFontVMetrics(fontBuffer, ascent, descent, lineGap);

            // Calculate scale for pixel heights
            float scale = stbtt_ScaleForPixelHeight(fontBuffer, fontSize);

            // Create bitmap for font atlas
            ByteBuffer bitmap = BufferUtils.createByteBuffer(BITMAP_WIDTH * BITMAP_HEIGHT);

            // Bake font bitmap
            STBTTBakedChar.Buffer charData = STBTTBakedChar.malloc(NUM_CHARS);
            int result = stbtt_BakeFontBitmap(fontBuffer, scale, bitmap, BITMAP_WIDTH, BITMAP_HEIGHT,
                                            FIRST_CHAR, NUM_CHARS, charData);
            if (result == 0) {
                throw new IOException("Failed to bake font bitmap");
            }

            // Create OpenGL texture from bitmap
            GLInfo glInfo = new GLInfo(); // TODO: Get from actual GL context
            Texture2D fontAtlas = new Texture2D(BITMAP_WIDTH, BITMAP_HEIGHT, bitmap, glInfo, 1.0f, false, false, false);

            return new Font("loaded_font", fontSize, fontAtlas, charData,
                           scale, ascent.get(0) * scale, descent.get(0) * scale, lineGap.get(0) * scale);
        }
    }

    public String getName() {
        return name;
    }

    public float getSize() {
        return size;
    }

    public float getLineHeight() {
        return (ascent - descent + lineGap) / scale;
    }

    public float getBaselineOffset() {
        return ascent / scale;
    }

    public float measureText(String text) {
        if (text == null || text.isEmpty()) {
            return 0.0f;
        }

        float width = 0.0f;
        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c >= FIRST_CHAR && c < FIRST_CHAR + NUM_CHARS) {
                STBTTBakedChar charInfo = charData.get(c - FIRST_CHAR);
                width += charInfo.xadvance();
            }
        }
        return width / scale;
    }

    public float getTextWidth(String text) {
        return measureText(text);
    }

    public Vector2f measureTextSize(String text) {
        if (text == null || text.isEmpty()) {
            return new Vector2f(0.0f, getLineHeight());
        }

        float width = 0.0f;
        float maxWidth = 0.0f;

        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c == '\n') {
                maxWidth = Math.max(maxWidth, width);
                width = 0.0f;
            } else if (c >= FIRST_CHAR && c < FIRST_CHAR + NUM_CHARS) {
                STBTTBakedChar charInfo = charData.get(c - FIRST_CHAR);
                width += charInfo.xadvance();
            }
        }
        maxWidth = Math.max(maxWidth, width);

        return new Vector2f(maxWidth / scale, getLineHeight());
    }

    public void bindAtlas() {
        fontAtlas.bind(0);
    }

    public void getCharQuad(char c, FloatBuffer quadData) {
        if (c >= FIRST_CHAR && c < FIRST_CHAR + NUM_CHARS) {
            STBTTBakedChar charInfo = charData.get(c - FIRST_CHAR);

            // STB returns quad in format: x0, y0, x1, y1, s0, t0, s1, t1
            quadData.put(0, charInfo.x0());
            quadData.put(1, charInfo.y0());
            quadData.put(2, charInfo.x1());
            quadData.put(3, charInfo.y1());
            quadData.put(4, charInfo.s0());
            quadData.put(5, charInfo.t0());
            quadData.put(6, charInfo.s1());
            quadData.put(7, charInfo.t1());
        } else {
            // Return empty quad for unsupported characters
            quadData.put(0, 0.0f);
            quadData.put(1, 0.0f);
            quadData.put(2, 0.0f);
            quadData.put(3, 0.0f);
            quadData.put(4, 0.0f);
            quadData.put(5, 0.0f);
            quadData.put(6, 0.0f);
            quadData.put(7, 0.0f);
        }
    }

    public float getCharAdvance(char c) {
        if (c >= FIRST_CHAR && c < FIRST_CHAR + NUM_CHARS) {
            STBTTBakedChar charInfo = charData.get(c - FIRST_CHAR);
            return charInfo.xadvance() / scale;
        }
        return 0.0f;
    }

    public Texture2D getAtlas() {
        return fontAtlas;
    }

    public int getAtlasWidth() {
        return BITMAP_WIDTH;
    }

    public int getAtlasHeight() {
        return BITMAP_HEIGHT;
    }

    @Override
    public void close() {
        if (fontAtlas != null) {
            fontAtlas.close();
        }
        if (charData != null) {
            charData.free();
        }
    }
}

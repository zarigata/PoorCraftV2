package com.poorcraft.client.render.texture;

import com.poorcraft.client.render.GLInfo;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;
import org.slf4j.Logger;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Path;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL12.GL_CLAMP_TO_EDGE;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL30.glGenerateMipmap;
import static org.lwjgl.opengl.GL42.glTexStorage2D;
import static org.lwjgl.opengl.GL45.*;
import static org.lwjgl.opengl.EXTTextureFilterAnisotropic.GL_TEXTURE_MAX_ANISOTROPY_EXT;

/**
 * Manages 2D texture loading and OpenGL texture objects.
 */
public class Texture2D implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(Texture2D.class);
    
    private int textureId;
    private int width;
    private int height;
    private final boolean useDSA;
    
    /**
     * Creates a texture from a file path.
     * 
     * @param path The path to the image file
     * @param capabilities OpenGL capabilities for feature detection
     * @param anisotropy Anisotropic filtering level (1.0 = disabled)
     * @param flipVertically Whether to flip the texture vertically on load
     * @param srgb Whether to use sRGB color space
     * @param generateMipmaps Whether to generate mipmaps
     */
    public Texture2D(Path path, GLInfo capabilities, float anisotropy, boolean flipVertically, boolean srgb, boolean generateMipmaps) {
        this.useDSA = capabilities.hasDirectStateAccess();
        loadFromFile(path, capabilities, anisotropy, flipVertically, srgb, generateMipmaps);
    }
    
    /**
     * Creates a texture from raw data.
     * 
     * @param width Texture width
     * @param height Texture height
     * @param data Texture data (RGBA format)
     * @param capabilities OpenGL capabilities
     * @param anisotropy Anisotropic filtering level (1.0 = disabled)
     * @param flipVertically Whether to flip the texture vertically (ignored for raw data)
     * @param srgb Whether to use sRGB color space
     * @param generateMipmaps Whether to generate mipmaps
     */
    public Texture2D(int width, int height, ByteBuffer data, GLInfo capabilities, float anisotropy, boolean flipVertically, boolean srgb, boolean generateMipmaps) {
        this.useDSA = capabilities.hasDirectStateAccess();
        this.width = width;
        this.height = height;
        createTexture(data, capabilities, anisotropy, srgb, generateMipmaps);
    }
    
    /**
     * Loads texture from a file.
     */
    private void loadFromFile(Path path, GLInfo capabilities, float anisotropy, boolean flipVertically, boolean srgb, boolean generateMipmaps) {
        // Flip texture vertically if requested (OpenGL expects bottom-left origin)
        STBImage.stbi_set_flip_vertically_on_load(flipVertically);
        
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer widthBuf = stack.mallocInt(1);
            IntBuffer heightBuf = stack.mallocInt(1);
            IntBuffer channelsBuf = stack.mallocInt(1);
            
            // Load image with 4 channels (RGBA)
            ByteBuffer imageData = STBImage.stbi_load(path.toString(), widthBuf, heightBuf, channelsBuf, 4);
            
            if (imageData == null) {
                String error = STBImage.stbi_failure_reason();
                LOGGER.error("Failed to load texture {}: {}", path, error);
                throw new RuntimeException("Failed to load texture: " + error);
            }
            
            this.width = widthBuf.get(0);
            this.height = heightBuf.get(0);
            
            LOGGER.debug("Loaded texture: {} ({}x{}, {} channels)", path, width, height, channelsBuf.get(0));
            
            createTexture(imageData, capabilities, anisotropy, srgb, generateMipmaps);
            
            // Free image data
            STBImage.stbi_image_free(imageData);
        }
    }
    
    /**
     * Creates the OpenGL texture object.
     */
    private void createTexture(ByteBuffer data, GLInfo capabilities, float anisotropy, boolean srgb, boolean generateMipmaps) {
        int internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        int mipLevels = generateMipmaps ? calculateMipLevels(width, height) : 1;
        
        if (useDSA) {
            // Direct State Access path (OpenGL 4.5+)
            textureId = glCreateTextures(GL_TEXTURE_2D);
            
            // Allocate immutable storage
            glTextureStorage2D(textureId, mipLevels, internalFormat, width, height);
            
            // Upload texture data
            glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            
            // Generate mipmaps
            if (generateMipmaps) {
                glGenerateTextureMipmap(textureId);
            }
            
            // Set texture parameters
            glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            // Apply anisotropic filtering if supported and requested
            if (capabilities.hasAnisotropicFiltering() && anisotropy > 1.0f) {
                float actualAniso = Math.min(capabilities.getMaxAnisotropy(), anisotropy);
                glTextureParameterf(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, actualAniso);
            }
            
        } else {
            // Traditional path (OpenGL 3.3+)
            textureId = glGenTextures();
            glBindTexture(GL_TEXTURE_2D, textureId);
            
            // Upload texture data
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            
            // Generate mipmaps
            if (generateMipmaps) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            
            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            // Apply anisotropic filtering if supported and requested
            if (capabilities.hasAnisotropicFiltering() && anisotropy > 1.0f) {
                float actualAniso = Math.min(capabilities.getMaxAnisotropy(), anisotropy);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, actualAniso);
            }
            
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        LOGGER.debug("Created texture (ID: {}, {}x{}, {} mip levels)", textureId, width, height, mipLevels);
    }
    
    /**
     * Calculates the number of mipmap levels.
     */
    private int calculateMipLevels(int width, int height) {
        int maxDimension = Math.max(width, height);
        return 1 + (int) Math.floor(Math.log(maxDimension) / Math.log(2));
    }
    
    /**
     * Binds the texture to a texture unit.
     * 
     * @param unit The texture unit (0-based)
     */
    public void bind(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
    
    /**
     * Reloads the texture from a file.
     * 
     * @param path The path to the new image
     * @param capabilities OpenGL capabilities
     * @param anisotropy Anisotropic filtering level
     * @param flipVertically Whether to flip the texture vertically
     * @param srgb Whether to use sRGB color space
     * @param generateMipmaps Whether to generate mipmaps
     */
    public void reload(Path path, GLInfo capabilities, float anisotropy, boolean flipVertically, boolean srgb, boolean generateMipmaps) {
        int oldId = textureId;
        try {
            loadFromFile(path, capabilities, anisotropy, flipVertically, srgb, generateMipmaps);
            glDeleteTextures(oldId);
            LOGGER.info("Reloaded texture: {}", path);
        } catch (Exception e) {
            LOGGER.error("Failed to reload texture, keeping old version", e);
            textureId = oldId;
        }
    }
    
    public int getWidth() {
        return width;
    }
    
    public int getHeight() {
        return height;
    }
    
    public int getId() {
        return textureId;
    }
    
    @Override
    public void close() {
        if (textureId != 0) {
            glDeleteTextures(textureId);
            textureId = 0;
        }
    }
}

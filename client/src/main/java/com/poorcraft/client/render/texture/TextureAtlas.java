package com.poorcraft.client.render.texture;

import com.poorcraft.client.render.GLInfo;
import org.joml.Vector4f;
import org.lwjgl.stb.STBImage;
import org.lwjgl.system.MemoryStack;
import org.slf4j.Logger;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL12.*;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL30.*;
import static org.lwjgl.opengl.GL42.glTexStorage3D;
import static org.lwjgl.opengl.GL45.*;
import static org.lwjgl.opengl.EXTTextureFilterAnisotropic.GL_TEXTURE_MAX_ANISOTROPY_EXT;

/**
 * Manages multiple textures in a single GL_TEXTURE_2D_ARRAY for efficient rendering.
 */
public class TextureAtlas implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(TextureAtlas.class);
    
    private int textureId;
    private final int tileSize;
    private final int maxLayers;
    private int currentLayer;
    private final boolean useDSA;
    
    private final Map<String, Integer> layerMap;
    
    /**
     * Creates a new texture atlas.
     * 
     * @param tileSize Size of each texture tile (e.g., 16x16)
     * @param maxLayers Maximum number of layers in the array
     * @param capabilities OpenGL capabilities
     */
    public TextureAtlas(int tileSize, int maxLayers, GLInfo capabilities) {
        this.tileSize = tileSize;
        this.maxLayers = maxLayers;
        this.currentLayer = 0;
        this.useDSA = capabilities.hasDirectStateAccess();
        this.layerMap = new HashMap<>();
        
        createAtlas(capabilities);
    }
    
    /**
     * Creates the texture array.
     */
    private void createAtlas(GLInfo capabilities) {
        int mipLevels = 1 + (int) Math.floor(Math.log(tileSize) / Math.log(2));
        
        if (useDSA) {
            // Direct State Access path
            textureId = glCreateTextures(GL_TEXTURE_2D_ARRAY);
            glTextureStorage3D(textureId, mipLevels, GL_RGBA8, tileSize, tileSize, maxLayers);
            
            // Set texture parameters
            glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            if (capabilities.hasAnisotropicFiltering()) {
                float maxAniso = Math.min(capabilities.getMaxAnisotropy(), 8.0f);
                glTextureParameterf(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
            }
            
        } else {
            // Traditional path
            textureId = glGenTextures();
            glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, tileSize, tileSize, maxLayers);
            
            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            if (capabilities.hasAnisotropicFiltering()) {
                float maxAniso = Math.min(capabilities.getMaxAnisotropy(), 8.0f);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
            }
            
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
        
        LOGGER.info("Created texture atlas: {}x{} tiles, {} max layers", tileSize, tileSize, maxLayers);
    }
    
    /**
     * Adds a texture to the atlas.
     * 
     * @param name The name to associate with this texture
     * @param path The path to the texture file
     * @return The layer index, or -1 if failed
     */
    public int addTexture(String name, Path path) {
        if (currentLayer >= maxLayers) {
            LOGGER.error("Texture atlas is full, cannot add: {}", name);
            return -1;
        }
        
        // Check if already added
        if (layerMap.containsKey(name)) {
            LOGGER.warn("Texture already in atlas: {}", name);
            return layerMap.get(name);
        }
        
        // Load image
        STBImage.stbi_set_flip_vertically_on_load(true);
        
        try (MemoryStack stack = MemoryStack.stackPush()) {
            IntBuffer widthBuf = stack.mallocInt(1);
            IntBuffer heightBuf = stack.mallocInt(1);
            IntBuffer channelsBuf = stack.mallocInt(1);
            
            ByteBuffer imageData = STBImage.stbi_load(path.toString(), widthBuf, heightBuf, channelsBuf, 4);
            
            if (imageData == null) {
                LOGGER.error("Failed to load texture {}: {}", path, STBImage.stbi_failure_reason());
                return -1;
            }
            
            int width = widthBuf.get(0);
            int height = heightBuf.get(0);
            
            // Validate dimensions
            if (width != tileSize || height != tileSize) {
                LOGGER.error("Texture {} has wrong dimensions ({}x{}, expected {}x{})", 
                    name, width, height, tileSize, tileSize);
                STBImage.stbi_image_free(imageData);
                return -1;
            }
            
            // Upload to layer
            if (useDSA) {
                glTextureSubImage3D(textureId, 0, 0, 0, currentLayer, tileSize, tileSize, 1, 
                    GL_RGBA, GL_UNSIGNED_BYTE, imageData);
            } else {
                glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, currentLayer, tileSize, tileSize, 1, 
                    GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }
            
            STBImage.stbi_image_free(imageData);
            
            layerMap.put(name, currentLayer);
            LOGGER.debug("Added texture to atlas: {} -> layer {}", name, currentLayer);
            
            return currentLayer++;
            
        } catch (Exception e) {
            LOGGER.error("Failed to add texture to atlas: {}", name, e);
            return -1;
        }
    }
    
    /**
     * Generates mipmaps for the atlas.
     * Should be called after all textures are added.
     */
    public void generateMipmaps() {
        if (useDSA) {
            glGenerateTextureMipmap(textureId);
        } else {
            glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
        LOGGER.debug("Generated mipmaps for texture atlas");
    }
    
    /**
     * Gets the layer index for a texture name.
     * 
     * @param name The texture name
     * @return The layer index, or -1 if not found
     */
    public int getLayer(String name) {
        return layerMap.getOrDefault(name, -1);
    }
    
    /**
     * Binds the atlas to a texture unit.
     * 
     * @param unit The texture unit (0-based)
     */
    public void bind(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureId);
    }
    
    public int getTileSize() {
        return tileSize;
    }
    
    public int getLayerCount() {
        return currentLayer;
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

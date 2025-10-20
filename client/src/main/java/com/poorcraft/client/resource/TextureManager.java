package com.poorcraft.client.resource;

import com.poorcraft.client.render.GLInfo;
import com.poorcraft.client.render.texture.Texture2D;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.core.resource.ResourceManager;
import org.lwjgl.system.MemoryUtil;
import org.slf4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Manages texture loading and hot-reloading.
 */
public class TextureManager extends ResourceManager<Texture2D> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(TextureManager.class);
    
    private final Path texturePath;
    private final GLInfo capabilities;
    private final Configuration config;
    private Texture2D missingTexture;
    
    /**
     * Creates a new TextureManager.
     * 
     * @param config Configuration for texture path
     * @param capabilities OpenGL capabilities
     */
    public TextureManager(Configuration config, GLInfo capabilities) {
        super();
        String textureDir = config.getString("resources.texturePath", "textures");
        this.texturePath = Path.of(textureDir);
        this.capabilities = capabilities;
        this.config = config;
        
        // Create missing texture
        createMissingTexture();
        
        LOGGER.info("Texture manager initialized (path: {})", texturePath);
    }
    
    @Override
    protected Texture2D loadResource(Path absolutePath) throws IOException {
        try {
            float anisotropy = config.getFloat("graphics.anisotropicFiltering", 8.0f);
            boolean flipTextures = config.getBoolean("graphics.flipTextures", true);
            
            // Try filesystem first
            if (Files.exists(absolutePath)) {
                return new Texture2D(absolutePath, capabilities, anisotropy, flipTextures, true, true);
            }
            
            // Try classpath fallback
            String filename = absolutePath.getFileName().toString();
            try (InputStream is = getClass().getClassLoader().getResourceAsStream("textures/" + filename)) {
                if (is != null) {
                    LOGGER.info("Loading texture from classpath: {}", filename);
                    return loadFromStream(is, capabilities, anisotropy, flipTextures);
                }
            }
            
            LOGGER.warn("Texture not found on filesystem or classpath: {}", absolutePath);
            return missingTexture;
        } catch (Exception e) {
            LOGGER.error("Failed to load texture: {}", absolutePath, e);
            return missingTexture;
        }
    }
    
    @Override
    protected void unloadResource(Texture2D resource) {
        if (resource != null && resource != missingTexture) {
            resource.close();
        }
    }
    
    @Override
    protected Path resolvePath(String path) {
        return texturePath.resolve(path);
    }
    
    /**
     * Loads a texture by name.
     * 
     * @param name Texture filename
     * @return The loaded texture
     * @throws IOException if loading fails
     */
    public Texture2D loadTexture(String name) throws IOException {
        return load(name);
    }
    
    /**
     * Gets a loaded texture by name.
     * 
     * @param name Texture name
     * @return The texture, or missing texture if not found
     */
    public Texture2D getTexture(String name) {
        Texture2D texture = get(name);
        return texture != null ? texture : missingTexture;
    }
    
    /**
     * Reloads a texture from disk.
     * 
     * @param name Texture name
     * @throws IOException if reload fails
     */
    public void reloadTexture(String name) throws IOException {
        reload(name);
    }
    
    /**
     * Creates a procedural missing texture (magenta/black checkerboard).
     */
    private void createMissingTexture() {
        int size = 16;
        int tileSize = 8;
        ByteBuffer data = MemoryUtil.memAlloc(size * size * 4);
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                boolean isMagenta = ((x / tileSize) + (y / tileSize)) % 2 == 0;
                
                if (isMagenta) {
                    data.put((byte) 255); // R
                    data.put((byte) 0);   // G
                    data.put((byte) 255); // B
                    data.put((byte) 255); // A
                } else {
                    data.put((byte) 0);   // R
                    data.put((byte) 0);   // G
                    data.put((byte) 0);   // B
                    data.put((byte) 255); // A
                }
            }
        }
        
        data.flip();
        
        // Missing texture uses defaults: no anisotropy, no flip, no sRGB, no mipmaps
        missingTexture = new Texture2D(size, size, data, capabilities, 1.0f, false, false, false);
        
        MemoryUtil.memFree(data);
        
        LOGGER.debug("Created missing texture");
    }
    
    /**
     * Loads a texture from an input stream.
     */
    private Texture2D loadFromStream(InputStream is, GLInfo capabilities, float anisotropy, boolean flipTextures) throws IOException {
        // Read all bytes from stream
        byte[] bytes = is.readAllBytes();
        ByteBuffer buffer = MemoryUtil.memAlloc(bytes.length);
        buffer.put(bytes);
        buffer.flip();
        
        try {
            // Use STBImage to load from memory
            org.lwjgl.stb.STBImage.stbi_set_flip_vertically_on_load(flipTextures);
            
            org.lwjgl.system.MemoryStack stack = org.lwjgl.system.MemoryStack.stackPush();
            try {
                java.nio.IntBuffer widthBuf = stack.mallocInt(1);
                java.nio.IntBuffer heightBuf = stack.mallocInt(1);
                java.nio.IntBuffer channelsBuf = stack.mallocInt(1);
                
                ByteBuffer imageData = org.lwjgl.stb.STBImage.stbi_load_from_memory(buffer, widthBuf, heightBuf, channelsBuf, 4);
                
                if (imageData == null) {
                    throw new IOException("Failed to decode image: " + org.lwjgl.stb.STBImage.stbi_failure_reason());
                }
                
                int width = widthBuf.get(0);
                int height = heightBuf.get(0);
                
                Texture2D texture = new Texture2D(width, height, imageData, capabilities, anisotropy, false, true, true);
                
                org.lwjgl.stb.STBImage.stbi_image_free(imageData);
                stack.pop();
                
                return texture;
            } catch (Exception e) {
                stack.pop();
                throw e;
            }
        } finally {
            MemoryUtil.memFree(buffer);
        }
    }
    
    /**
     * Gets the missing texture.
     * 
     * @return The missing texture
     */
    public Texture2D getMissingTexture() {
        return missingTexture;
    }

    public GLInfo getCapabilities() {
        return capabilities;
    }

    public Configuration getConfig() {
        return config;
    }
    
    @Override
    public void close() {
        super.close();
        if (missingTexture != null) {
            missingTexture.close();
            missingTexture = null;
        }
    }
}

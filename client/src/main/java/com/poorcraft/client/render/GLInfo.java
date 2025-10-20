package com.poorcraft.client.render;

import org.lwjgl.opengl.GL;
import org.slf4j.Logger;

import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL30.*;

/**
 * Detects and stores OpenGL capabilities at runtime.
 */
public class GLInfo {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(GLInfo.class);
    
    // Version and vendor info
    private final String renderer;
    private final String vendor;
    private final String version;
    private final String glslVersion;
    
    // Feature support
    private final boolean openGL45;
    private final boolean directStateAccess;
    private final boolean bufferStorage;
    private final boolean clipControl;
    private final boolean separateShaderObjects;
    private final boolean multiDrawIndirect;
    private final boolean shaderStorageBuffer;
    private final boolean anisotropicFiltering;
    private final boolean debugOutput;
    
    // Limits
    private final int maxTextureSize;
    private final int maxTextureUnits;
    private final int maxUBOSize;
    private final int maxSSBOSize;
    private final int uboOffsetAlignment;
    private final float maxAnisotropy;
    
    /**
     * Creates a new GLInfo instance by querying OpenGL.
     */
    public GLInfo() {
        // Get version and vendor info
        this.renderer = glGetString(GL_RENDERER);
        this.vendor = glGetString(GL_VENDOR);
        this.version = glGetString(GL_VERSION);
        this.glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        
        // Get LWJGL capabilities
        org.lwjgl.opengl.GLCapabilities caps = GL.getCapabilities();
        
        // Detect features
        this.openGL45 = caps.OpenGL45;
        this.directStateAccess = caps.GL_ARB_direct_state_access || caps.OpenGL45;
        this.bufferStorage = caps.GL_ARB_buffer_storage || caps.OpenGL44;
        this.clipControl = caps.GL_ARB_clip_control || caps.OpenGL45;
        this.separateShaderObjects = caps.GL_ARB_separate_shader_objects || caps.OpenGL41;
        this.multiDrawIndirect = caps.GL_ARB_multi_draw_indirect || caps.OpenGL43;
        this.shaderStorageBuffer = caps.GL_ARB_shader_storage_buffer_object || caps.OpenGL43;
        this.anisotropicFiltering = caps.GL_EXT_texture_filter_anisotropic;
        this.debugOutput = caps.GL_KHR_debug || caps.GL_ARB_debug_output;
        
        // Query limits
        this.maxTextureSize = glGetInteger(GL_MAX_TEXTURE_SIZE);
        this.maxTextureUnits = glGetInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        this.maxUBOSize = glGetInteger(0x8A2B); // GL_MAX_UNIFORM_BLOCK_SIZE
        this.maxSSBOSize = shaderStorageBuffer ? glGetInteger(0x90D5) : 0; // GL_MAX_SHADER_STORAGE_BLOCK_SIZE
        this.uboOffsetAlignment = glGetInteger(0x8A34); // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
        this.maxAnisotropy = anisotropicFiltering ? glGetFloat(0x84FF) : 1.0f; // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    }
    
    /**
     * Prints all detected capabilities and limits to the log.
     */
    public void printCapabilities() {
        LOGGER.info("=== OpenGL Capabilities ===");
        LOGGER.info("Renderer: {}", renderer);
        LOGGER.info("Vendor: {}", vendor);
        LOGGER.info("Version: {}", version);
        LOGGER.info("GLSL Version: {}", glslVersion);
        LOGGER.info("");
        LOGGER.info("Features:");
        LOGGER.info("  OpenGL 4.5: {}", openGL45);
        LOGGER.info("  Direct State Access: {}", directStateAccess);
        LOGGER.info("  Buffer Storage: {}", bufferStorage);
        LOGGER.info("  Clip Control: {}", clipControl);
        LOGGER.info("  Separate Shader Objects: {}", separateShaderObjects);
        LOGGER.info("  Multi Draw Indirect: {}", multiDrawIndirect);
        LOGGER.info("  Shader Storage Buffer: {}", shaderStorageBuffer);
        LOGGER.info("  Anisotropic Filtering: {}", anisotropicFiltering);
        LOGGER.info("  Debug Output: {}", debugOutput);
        LOGGER.info("");
        LOGGER.info("Limits:");
        LOGGER.info("  Max Texture Size: {}", maxTextureSize);
        LOGGER.info("  Max Texture Units: {}", maxTextureUnits);
        LOGGER.info("  Max UBO Size: {} bytes", maxUBOSize);
        LOGGER.info("  Max SSBO Size: {} bytes", maxSSBOSize);
        LOGGER.info("  UBO Offset Alignment: {} bytes", uboOffsetAlignment);
        LOGGER.info("  Max Anisotropy: {}x", maxAnisotropy);
        LOGGER.info("===========================");
    }
    
    // Getters
    
    public String getRenderer() { return renderer; }
    public String getVendor() { return vendor; }
    public String getVersion() { return version; }
    public String getGlslVersion() { return glslVersion; }
    
    public boolean isOpenGL45() { return openGL45; }
    public boolean hasDirectStateAccess() { return directStateAccess; }
    public boolean hasBufferStorage() { return bufferStorage; }
    public boolean hasClipControl() { return clipControl; }
    public boolean hasSeparateShaderObjects() { return separateShaderObjects; }
    public boolean hasMultiDrawIndirect() { return multiDrawIndirect; }
    public boolean hasShaderStorageBuffer() { return shaderStorageBuffer; }
    public boolean hasAnisotropicFiltering() { return anisotropicFiltering; }
    public boolean hasDebugOutput() { return debugOutput; }
    
    public int getMaxTextureSize() { return maxTextureSize; }
    public int getMaxTextureUnits() { return maxTextureUnits; }
    public int getMaxUBOSize() { return maxUBOSize; }
    public int getMaxSSBOSize() { return maxSSBOSize; }
    public int getUboOffsetAlignment() { return uboOffsetAlignment; }
    public float getMaxAnisotropy() { return maxAnisotropy; }
}

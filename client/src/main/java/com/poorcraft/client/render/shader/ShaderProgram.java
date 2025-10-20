package com.poorcraft.client.render.shader;

import org.joml.Matrix4f;
import org.joml.Vector3f;
import org.joml.Vector4f;
import org.lwjgl.system.MemoryStack;
import org.slf4j.Logger;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import static org.lwjgl.opengl.GL20.*;
import static org.lwjgl.opengl.GL32.GL_GEOMETRY_SHADER;

/**
 * Manages GLSL shader compilation, linking, and uniform management.
 */
public class ShaderProgram implements AutoCloseable {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ShaderProgram.class);
    
    private int programId;
    private final Map<String, Integer> uniformLocations;
    
    /**
     * Creates a shader program from vertex and fragment shader source code.
     * 
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @throws IOException if compilation or linking fails
     */
    public ShaderProgram(String vertexSource, String fragmentSource) throws IOException {
        this.uniformLocations = new ConcurrentHashMap<>();
        compile(vertexSource, fragmentSource, null);
    }
    
    /**
     * Creates a shader program from vertex, fragment, and optional geometry shader source.
     * 
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @param geometrySource Geometry shader source code (can be null)
     * @throws IOException if compilation or linking fails
     */
    public ShaderProgram(String vertexSource, String fragmentSource, String geometrySource) throws IOException {
        this.uniformLocations = new ConcurrentHashMap<>();
        compile(vertexSource, fragmentSource, geometrySource);
    }
    
    /**
     * Creates a shader program from file paths.
     * 
     * @param vertexPath Path to vertex shader
     * @param fragmentPath Path to fragment shader
     * @return The compiled shader program
     * @throws IOException if files cannot be read or compilation fails
     */
    public static ShaderProgram fromFiles(Path vertexPath, Path fragmentPath) throws IOException {
        String vertexSource = Files.readString(vertexPath);
        String fragmentSource = Files.readString(fragmentPath);
        return new ShaderProgram(vertexSource, fragmentSource);
    }
    
    /**
     * Compiles and links shaders.
     */
    private void compile(String vertexSource, String fragmentSource, String geometrySource) throws IOException {
        int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        int geometryShader = 0;
        
        if (geometrySource != null) {
            geometryShader = compileShader(GL_GEOMETRY_SHADER, geometrySource);
        }
        
        // Create program
        programId = glCreateProgram();
        glAttachShader(programId, vertexShader);
        glAttachShader(programId, fragmentShader);
        if (geometryShader != 0) {
            glAttachShader(programId, geometryShader);
        }
        
        // Link program
        glLinkProgram(programId);
        
        // Check link status
        if (glGetProgrami(programId, GL_LINK_STATUS) == GL_FALSE) {
            String log = glGetProgramInfoLog(programId);
            glDeleteProgram(programId);
            throw new IOException("Shader program linking failed:\n" + log);
        }
        
        // Validate program (optional, for debugging)
        glValidateProgram(programId);
        if (glGetProgrami(programId, GL_VALIDATE_STATUS) == GL_FALSE) {
            LOGGER.warn("Shader program validation failed:\n{}", glGetProgramInfoLog(programId));
        }
        
        // Detach and delete shaders
        glDetachShader(programId, vertexShader);
        glDetachShader(programId, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        if (geometryShader != 0) {
            glDetachShader(programId, geometryShader);
            glDeleteShader(geometryShader);
        }
        
        LOGGER.debug("Shader program compiled and linked successfully (ID: {})", programId);
    }
    
    /**
     * Compiles a single shader.
     */
    private int compileShader(int type, String source) throws IOException {
        int shader = glCreateShader(type);
        glShaderSource(shader, source);
        glCompileShader(shader);
        
        // Check compile status
        if (glGetShaderi(shader, GL_COMPILE_STATUS) == GL_FALSE) {
            String log = glGetShaderInfoLog(shader);
            glDeleteShader(shader);
            String typeName = getShaderTypeName(type);
            throw new IOException(typeName + " shader compilation failed:\n" + log);
        }
        
        return shader;
    }
    
    /**
     * Gets a human-readable shader type name.
     */
    private String getShaderTypeName(int type) {
        return switch (type) {
            case GL_VERTEX_SHADER -> "Vertex";
            case GL_FRAGMENT_SHADER -> "Fragment";
            case GL_GEOMETRY_SHADER -> "Geometry";
            default -> "Unknown";
        };
    }
    
    /**
     * Reloads the shader with new source code.
     * Only swaps the program ID on success.
     * 
     * @param vertexSource New vertex shader source
     * @param fragmentSource New fragment shader source
     */
    public void reload(String vertexSource, String fragmentSource) {
        try {
            int oldProgramId = programId;
            compile(vertexSource, fragmentSource, null);
            glDeleteProgram(oldProgramId);
            uniformLocations.clear();
            LOGGER.info("Shader program reloaded successfully");
        } catch (IOException e) {
            LOGGER.error("Failed to reload shader program, keeping old version", e);
        }
    }
    
    /**
     * Binds this shader program for use.
     */
    public void use() {
        glUseProgram(programId);
    }
    
    /**
     * Gets or caches a uniform location.
     */
    private int getUniformLocation(String name) {
        return uniformLocations.computeIfAbsent(name, n -> {
            int location = glGetUniformLocation(programId, n);
            if (location == -1) {
                LOGGER.warn("Uniform '{}' not found in shader program", n);
            }
            return location;
        });
    }
    
    // Uniform setters
    
    public void setInt(String name, int value) {
        glUniform1i(getUniformLocation(name), value);
    }
    
    public void setFloat(String name, float value) {
        glUniform1f(getUniformLocation(name), value);
    }
    
    public void setVector2f(String name, float x, float y) {
        glUniform2f(getUniformLocation(name), x, y);
    }
    
    public void setVector3f(String name, Vector3f value) {
        glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
    }
    
    public void setVector3f(String name, float x, float y, float z) {
        glUniform3f(getUniformLocation(name), x, y, z);
    }
    
    public void setVector4f(String name, Vector4f value) {
        glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
    }
    
    public void setVector4f(String name, float x, float y, float z, float w) {
        glUniform4f(getUniformLocation(name), x, y, z, w);
    }
    
    public void setMatrix4f(String name, Matrix4f value) {
        try (MemoryStack stack = MemoryStack.stackPush()) {
            FloatBuffer buffer = stack.mallocFloat(16);
            value.get(buffer);
            glUniformMatrix4fv(getUniformLocation(name), false, buffer);
        }
    }
    
    public void setBoolean(String name, boolean value) {
        glUniform1i(getUniformLocation(name), value ? 1 : 0);
    }
    
    /**
     * Gets the program ID.
     * 
     * @return The OpenGL program ID
     */
    public int getProgramId() {
        return programId;
    }
    
    @Override
    public void close() {
        if (programId != 0) {
            glDeleteProgram(programId);
            programId = 0;
        }
    }
}

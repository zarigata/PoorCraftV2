package com.poorcraft.client.resource;

import com.poorcraft.client.render.shader.ShaderProgram;
import com.poorcraft.common.config.Configuration;
import com.poorcraft.core.resource.ResourceManager;
import org.slf4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

/**
 * Manages shader loading and hot-reloading.
 */
public class ShaderManager extends ResourceManager<ShaderProgram> {
    private static final Logger LOGGER = com.poorcraft.common.util.Logger.getLogger(ShaderManager.class);
    
    private final Path shaderPath;
    private final Map<String, Path> shaderFiles;
    
    /**
     * Creates a new ShaderManager.
     * 
     * @param config Configuration for shader path
     */
    public ShaderManager(Configuration config) {
        super();
        String shaderDir = config.getString("resources.shaderPath", "shaders");
        this.shaderPath = Path.of(shaderDir);
        this.shaderFiles = new HashMap<>();
        
        LOGGER.info("Shader manager initialized (path: {})", shaderPath);
    }
    
    @Override
    protected ShaderProgram loadResource(Path absolutePath) throws IOException {
        // Determine if this is a vertex or fragment shader
        String filename = absolutePath.getFileName().toString();
        String baseName = getBaseName(filename);
        
        // Find paired shaders
        Path vertPath = findShaderFile(baseName, ".vert");
        Path fragPath = findShaderFile(baseName, ".frag");
        
        String vertSource;
        String fragSource;
        
        // Try filesystem first, then classpath
        if (vertPath != null && Files.exists(vertPath)) {
            vertSource = Files.readString(vertPath);
        } else {
            vertSource = loadFromClasspath("shaders/" + baseName + ".vert");
            if (vertSource == null) {
                throw new IOException("Could not find vertex shader for: " + baseName);
            }
        }
        
        if (fragPath != null && Files.exists(fragPath)) {
            fragSource = Files.readString(fragPath);
        } else {
            fragSource = loadFromClasspath("shaders/" + baseName + ".frag");
            if (fragSource == null) {
                throw new IOException("Could not find fragment shader for: " + baseName);
            }
        }
        
        // Process includes (only if loaded from filesystem)
        if (vertPath != null && Files.exists(vertPath)) {
            vertSource = processIncludes(vertSource, vertPath.getParent());
        }
        if (fragPath != null && Files.exists(fragPath)) {
            fragSource = processIncludes(fragSource, fragPath.getParent());
        }
        
        // Create shader program
        ShaderProgram program = new ShaderProgram(vertSource, fragSource);
        
        // Store file paths for hot-reload (only if from filesystem)
        if (vertPath != null) {
            shaderFiles.put(baseName, vertPath);
        }
        if (fragPath != null) {
            shaderFiles.put(baseName + "_frag", fragPath);
        }
        
        return program;
    }
    
    /**
     * Loads a shader from the classpath.
     */
    private String loadFromClasspath(String resourcePath) {
        try (InputStream is = getClass().getClassLoader().getResourceAsStream(resourcePath)) {
            if (is == null) {
                return null;
            }
            return new String(is.readAllBytes(), StandardCharsets.UTF_8);
        } catch (IOException e) {
            LOGGER.warn("Failed to load shader from classpath: {}", resourcePath, e);
            return null;
        }
    }
    
    @Override
    protected void unloadResource(ShaderProgram resource) {
        resource.close();
    }
    
    @Override
    protected Path resolvePath(String path) {
        return shaderPath.resolve(path);
    }
    
    /**
     * Loads a shader by name.
     * 
     * @param name Shader name (without extension)
     * @return The loaded shader program
     * @throws IOException if loading fails
     */
    public ShaderProgram loadShader(String name) throws IOException {
        return load(name);
    }
    
    /**
     * Gets a loaded shader by name.
     * 
     * @param name Shader name
     * @return The shader program, or null if not loaded
     */
    public ShaderProgram getShader(String name) {
        return get(name);
    }
    
    /**
     * Reloads a shader from disk.
     * 
     * @param name Shader name
     * @throws IOException if reload fails
     */
    public void reloadShader(String name) throws IOException {
        reload(name);
    }
    
    /**
     * Gets the base name of a shader file (without extension).
     */
    private String getBaseName(String filename) {
        int dotIndex = filename.lastIndexOf('.');
        if (dotIndex > 0) {
            return filename.substring(0, dotIndex);
        }
        return filename;
    }
    
    /**
     * Finds a shader file by base name and extension.
     */
    private Path findShaderFile(String baseName, String extension) {
        Path path = shaderPath.resolve(baseName + extension);
        if (Files.exists(path)) {
            return path;
        }
        
        // Try with .glsl extension
        path = shaderPath.resolve(baseName + extension + ".glsl");
        if (Files.exists(path)) {
            return path;
        }
        
        return null;
    }
    
    /**
     * Processes #include directives in shader source.
     */
    private String processIncludes(String source, Path baseDir) throws IOException {
        StringBuilder result = new StringBuilder();
        String[] lines = source.split("\n");
        
        for (String line : lines) {
            String trimmed = line.trim();
            
            if (trimmed.startsWith("#include")) {
                // Extract include path
                int start = trimmed.indexOf('"');
                int end = trimmed.lastIndexOf('"');
                
                if (start >= 0 && end > start) {
                    String includePath = trimmed.substring(start + 1, end);
                    Path includeFile = baseDir.resolve(includePath);
                    
                    if (Files.exists(includeFile)) {
                        String includeSource = Files.readString(includeFile);
                        // Recursively process includes
                        includeSource = processIncludes(includeSource, includeFile.getParent());
                        result.append(includeSource).append("\n");
                    } else {
                        LOGGER.warn("Include file not found: {}", includePath);
                        result.append(line).append("\n");
                    }
                } else {
                    result.append(line).append("\n");
                }
            } else {
                result.append(line).append("\n");
            }
        }
        
        return result.toString();
    }
}

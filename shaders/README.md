# Shaders Directory

This directory contains all GLSL shader files used by the PoorCraft engine's rendering pipeline. Shaders are loaded and compiled at runtime by the ShaderManager system.

## Directory Structure

```
shaders/
├── basic/             # Simple shaders for testing and debugging
├── terrain/           # World rendering shaders with lighting and texturing
├── entity/            # Entity rendering shaders with skinning support
├── ui/                # 2D UI rendering shaders
├── post_processing/   # Screen-space effects (bloom, SSAO, etc.)
└── README.md          # This file
```

## Shader Organization

Shaders are organized by their primary purpose and complexity:

### Basic (`basic/`)

Contains simple shaders for testing and debugging:
- **vertex shaders**: Simple transformation and pass-through
- **fragment shaders**: Basic color output and texturing
- **Examples**: `passthrough.vert`, `color.frag`, `texture.frag`

### Terrain (`terrain/`)

Shaders for rendering the voxel world:
- **Chunk rendering**: Efficient rendering of large numbers of blocks
- **Lighting**: Support for directional lighting, ambient occlusion
- **Texturing**: Block texture atlases and blending
- **Level of Detail (LOD)**: Distance-based quality reduction
- **Examples**: `chunk.vert`, `chunk.frag`, `terrain_lighting.frag`

### Entity (`entity/`)

Shaders for rendering entities (players, mobs, items):
- **Skinning**: Support for animated models
- **32x32 texture support**: As per project specification
- **Lighting**: Dynamic lighting for entities
- **Transparency**: Support for transparent entity parts
- **Examples**: `entity.vert`, `entity.frag`, `skinned_entity.vert`

### UI (`ui/`)

Shaders for 2D user interface rendering:
- **Orthographic projection**: 2D rendering pipeline
- **Text rendering**: Font texture sampling and alpha blending
- **UI effects**: Hover effects, animations, transitions
- **Examples**: `ui.vert`, `ui.frag`, `text.frag`

### Post Processing (`post_processing/`)

Screen-space effects applied after main rendering:
- **Bloom**: Bright light bleeding effect
- **SSAO**: Screen Space Ambient Occlusion
- **HDR**: High Dynamic Range tone mapping
- **Motion blur**: Camera movement blur
- **Examples**: `bloom.vert`, `bloom.frag`, `ssao.frag`

## GLSL Version

All shaders should use OpenGL 4.6 Core profile:
```glsl
#version 460 core
```

## Shader Naming Convention

Shaders follow a consistent naming pattern:
- `descriptive_name.vert` - Vertex shaders
- `descriptive_name.frag` - Fragment shaders
- `descriptive_name.geom` - Geometry shaders
- `descriptive_name.tesc` - Tessellation control shaders
- `descriptive_name.tese` - Tessellation evaluation shaders
- `descriptive_name.comp` - Compute shaders

## Common Uniform Variables

### Camera and View
```glsl
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 cameraPosition;
uniform float nearPlane;
uniform float farPlane;
```

### Lighting
```glsl
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform int lightCount;
uniform Light lights[MAX_LIGHTS];
```

### Material Properties
```glsl
uniform vec3 materialColor;
uniform float shininess;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
```

### Time and Animation
```glsl
uniform float time;
uniform float deltaTime;
uniform int frameCount;
```

## Shader Loading

Shaders are loaded and managed by the ShaderManager:
- Automatic compilation and linking at runtime
- Error reporting for compilation failures
- Hot reloading during development
- Shader caching for improved load times

## Writing Shaders

### Best Practices
1. **Use modern GLSL features** (4.6 core)
2. **Organize code with functions** for readability
3. **Comment complex calculations** and algorithms
4. **Use consistent variable naming** (camelCase)
5. **Handle edge cases** (division by zero, NaN values)
6. **Optimize for performance** (minimize texture samples, use early exits)

### Performance Considerations
- **Minimize texture sampling** in fragment shaders
- **Use appropriate precision** (lowp, mediump, highp)
- **Batch similar operations** for better GPU utilization
- **Consider LOD techniques** for distant geometry
- **Profile shader performance** using GPU tools

### Debugging Shaders
- Use `gl_FragColor` for simple output debugging
- Comment out sections to isolate problems
- Check OpenGL error codes after shader operations
- Use RenderDoc or similar tools for GPU debugging

## Example Shader Structure

### Vertex Shader Example
```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
    TexCoord = aTexCoord;
}
```

### Fragment Shader Example
```glsl
#version 460 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Texture sampling
    vec4 texColor = texture(texture1, TexCoord);

    // Combine lighting
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColor = vec4(result, texColor.a);
}
```

## Adding New Shaders

1. Create shader files in the appropriate subdirectory
2. Follow the naming conventions
3. Implement proper GLSL 4.6 core syntax
4. Test compilation and linking
5. Add to shader loading system if needed

## Shader Compatibility

Shaders should be compatible with:
- **NVIDIA** graphics cards (GeForce series)
- **AMD** graphics cards (Radeon series)
- **Intel** integrated graphics
- Different driver versions and OpenGL implementations

## Resources

- **[OpenGL Shading Language Specification](https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language)**
- **[GLSL Tutorial](https://learnopengl.com/Getting-started/Shaders)**
- **[Shader Best Practices](https://developer.nvidia.com/gpu-programming-guide)**

For more information about the shader loading system, see the ShaderManager documentation.

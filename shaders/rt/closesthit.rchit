#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : require

// Ray payload
layout(location = 0) rayPayloadInEXT vec3 hitColor;

// Hit attributes (barycentric coordinates)
hitAttributeEXT vec2 attribs;

// Descriptor bindings
layout(set = 1, binding = 1) uniform sampler2D textures[];

// Camera uniform buffer
layout(set = 2, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    vec3 position;
    vec3 sunDirection;
    vec3 sunColor;
    float ambientStrength;
} camera;

// Vertex data (would be fetched from vertex buffer in real implementation)
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    float light;
    float ao;
};

void main() {
    // Interpolate vertex attributes using barycentric coordinates
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    
    // Placeholder: In real implementation, fetch vertex data from buffers
    // For now, use hit position and geometric normal
    vec3 worldPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3 normal = normalize(gl_ObjectToWorldEXT * vec4(gl_GeometryIndexEXT, 0, 0, 0)).xyz; // Placeholder
    
    // Simple lighting calculation
    float diffuse = max(0.0, dot(normal, camera.sunDirection));
    vec3 ambient = vec3(camera.ambientStrength);
    
    // Placeholder color (would sample texture in real implementation)
    vec3 baseColor = vec3(0.8, 0.8, 0.8);
    
    // Combine lighting
    vec3 finalColor = baseColor * (ambient + camera.sunColor * diffuse);
    
    hitColor = finalColor;
}

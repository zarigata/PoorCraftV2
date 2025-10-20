#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

// Ray payload
layout(location = 0) rayPayloadInEXT vec3 hitColor;

// Hit attributes (barycentric coordinates)
hitAttributeEXT vec2 attribs;

// Descriptor bindings
// Set 0, Binding 1: Camera uniform buffer
layout(set = 0, binding = 1) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    vec3 position;
    float _pad0;
    vec3 sunDirection;
    float _pad1;
    vec3 sunColor;
    float _pad2;
    vec3 skyTopColor;
    float _pad3;
    vec3 skyHorizonColor;
    float ambientStrength;
    float timeOfDay;
} camera;

void main() {
    // MVP: Simple placeholder color based on barycentric coordinates
    // This will be replaced when TLAS and geometry buffers are implemented
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    
    // Generate a simple color pattern
    vec3 color = vec3(0.8, 0.6, 0.4) * (barycentrics.x * 0.5 + 0.5);
    
    hitColor = color;
}

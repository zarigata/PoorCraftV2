#version 460
#extension GL_EXT_ray_tracing : require

// Ray payload
layout(location = 0) rayPayloadInEXT vec3 hitColor;

// Camera uniform buffer
layout(set = 2, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    vec3 position;
    vec3 sunDirection;
    vec3 sunColor;
    vec3 skyTopColor;
    vec3 skyHorizonColor;
    float timeOfDay;
} camera;

void main() {
    // Calculate sky color based on ray direction
    vec3 rayDir = gl_WorldRayDirectionEXT;
    
    // Gradient from horizon to zenith
    float gradientFactor = max(0.0, rayDir.y);
    vec3 skyColor = mix(camera.skyHorizonColor, camera.skyTopColor, gradientFactor);
    
    // Add sun contribution
    float sunDot = max(0.0, dot(rayDir, camera.sunDirection));
    if (sunDot > 0.99) {
        float sunIntensity = pow(sunDot, 100.0);
        skyColor += camera.sunColor * sunIntensity;
    }
    
    hitColor = skyColor;
}

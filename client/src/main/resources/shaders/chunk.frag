#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
in float vTexLayer;
in float vAO;

out vec4 FragColor;

uniform sampler2DArray uBlockAtlas;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;
uniform vec3 uCameraPos;

void main() {
    // Sample texture from array
    vec4 texColor = texture(uBlockAtlas, vec3(vTexCoord, vTexLayer));
    
    // Discard fully transparent pixels
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Calculate diffuse lighting
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(uSunDirection);
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Apply ambient occlusion
    float ao = vAO;
    
    // Combine lighting
    vec3 lighting = uAmbientColor + uSunColor * diffuse * ao;
    vec3 color = texColor.rgb * lighting;
    
    // Calculate fog
    float distance = length(vFragPos - uCameraPos);
    float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
    
    // Mix with fog color
    color = mix(uFogColor, color, fogFactor);
    
    FragColor = vec4(color, texColor.a);
}

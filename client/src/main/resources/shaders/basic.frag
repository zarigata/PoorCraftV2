#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    // Sample texture
    vec4 texColor = texture(uTexture, vTexCoord);
    
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(vNormal);
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Ambient + diffuse
    vec3 ambient = vec3(0.3);
    vec3 lighting = ambient + vec3(0.7) * diffuse;
    
    FragColor = vec4(texColor.rgb * lighting, texColor.a);
}

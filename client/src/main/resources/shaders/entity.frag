#version 330 core

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

uniform sampler2D uSkinTexture;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uCameraPos;
uniform float uFogStart;
uniform float uFogEnd;
uniform vec3 uFogColor;

void main() {
    vec4 texColor = texture(uSkinTexture, vTexCoord);
    if (texColor.a < 0.1) {
        discard;
    }

    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    vec3 ambient = uAmbientColor;
    vec3 lighting = ambient + diffuse;

    vec3 color = texColor.rgb * lighting;

    float distance = length(vFragPos - uCameraPos);
    float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
    color = mix(uFogColor, color, fogFactor);

    FragColor = vec4(color, texColor.a);
}

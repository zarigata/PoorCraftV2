#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vFragPos = worldPos.xyz;
    
    gl_Position = uProjection * uView * worldPos;
    
    vTexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
}

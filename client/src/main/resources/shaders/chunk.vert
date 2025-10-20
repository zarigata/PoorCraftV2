#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in float aTexLayer;
layout(location = 4) in float aAO;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;
out float vTexLayer;
out float vAO;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    gl_Position = uProjection * uView * worldPos;
    
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    vFragPos = worldPos.xyz;
    vTexLayer = aTexLayer;
    vAO = aAO;
}

#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aLight;
layout (location = 4) in uint aAO;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out float SkyLight;
out float BlockLight;
out float AO;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    // Apply wave animation
    vec3 pos = aPos;
    pos.y += sin(pos.x * 2.0 + time) * 0.05 + cos(pos.z * 2.0 + time) * 0.05;
    
    vec4 worldPosition = model * vec4(pos, 1.0);
    FragPos = worldPosition.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Animate UVs for flow effect
    TexCoord = aTexCoord + vec2(time * 0.05, time * 0.03);
    
    // Unpack and normalize light values
    SkyLight = float((aLight >> 4u) & 0x0Fu) / 15.0;
    BlockLight = float(aLight & 0x0Fu) / 15.0;
    AO = 1.0 - (float(aAO) / 3.0);
    
    gl_Position = projection * view * worldPosition;
}

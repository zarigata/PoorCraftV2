#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aInstancePos;
layout (location = 3) in vec4 aInstanceColor;
layout (location = 4) in float aInstanceSize;
layout (location = 5) in float aInstanceRotation;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;

void main()
{
    // Billboard quad facing camera
    vec3 vertexPos = aInstancePos + 
                     cameraRight * aPos.x * aInstanceSize + 
                     cameraUp * aPos.y * aInstanceSize;
    
    gl_Position = projection * view * vec4(vertexPos, 1.0);
    TexCoord = aTexCoord;
    ParticleColor = aInstanceColor;
}

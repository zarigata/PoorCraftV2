#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 SkyPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    SkyPos = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // Set depth to maximum (z = w) for infinite distance
    gl_Position = pos.xyww;
}

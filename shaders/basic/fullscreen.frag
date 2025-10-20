#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D inputTexture;

void main() {
    // Sample the input texture (RT output or other source)
    outColor = texture(inputTexture, fragUV);
}

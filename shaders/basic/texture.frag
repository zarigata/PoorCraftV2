#version 330 core

in vec2 TexCoord;

uniform sampler2D textureSampler;
uniform vec4 tintColor = vec4(1.0);

out vec4 FragColor;

void main()
{
    FragColor = texture(textureSampler, TexCoord) * tintColor;
}

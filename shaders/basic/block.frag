#version 460 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D blockAtlas;

void main()
{
    vec4 texColor = texture(blockAtlas, TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    FragColor = texColor;
}

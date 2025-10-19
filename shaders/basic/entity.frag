#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D skinTexture;

void main()
{
    vec4 texColor = texture(skinTexture, TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    FragColor = texColor;
}

#version 460 core

in vec2 TexCoord;
in vec4 ParticleColor;

out vec4 FragColor;

uniform sampler2D particleAtlas;

void main()
{
    vec4 texColor = texture(particleAtlas, TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    FragColor = texColor * ParticleColor;
}

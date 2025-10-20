#version 460 core

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in float SkyLight;
in float BlockLight;
in float AO;

out vec4 FragColor;

uniform sampler2D blockAtlas;
uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform float ambientStrength;
uniform vec4 waterColor;

void main()
{
    vec4 texColor = texture(blockAtlas, TexCoord);
    
    // Calculate lighting
    float ambient = max(SkyLight, ambientStrength);
    float diffuse = max(dot(normalize(Normal), sunDirection), 0.0) * SkyLight;
    float emissive = BlockLight;
    
    float totalLight = ambient + diffuse + emissive;
    totalLight *= AO;
    
    // Apply water tint and transparency
    vec3 finalColor = texColor.rgb * waterColor.rgb * sunColor * totalLight;
    FragColor = vec4(finalColor, waterColor.a);
}

#version 460 core

in vec3 SkyPos;

out vec4 FragColor;

uniform vec3 skyTopColor;
uniform vec3 skyHorizonColor;
uniform vec3 sunColor;
uniform vec3 moonColor;
uniform float timeOfDay;
uniform vec3 sunDirection;
uniform vec3 moonDirection;

void main()
{
    // Calculate gradient factor from vertical position
    float gradientFactor = (normalize(SkyPos).y + 1.0) * 0.5;
    vec3 skyColor = mix(skyHorizonColor, skyTopColor, gradientFactor);
    
    // Add sun disk
    float sunDot = dot(normalize(SkyPos), sunDirection);
    vec3 sunContribution = vec3(0.0);
    if (sunDot > 0.99) {
        float sunIntensity = (sunDot - 0.99) / 0.01;
        sunContribution = sunColor * sunIntensity;
    }
    
    // Add moon disk
    float moonDot = dot(normalize(SkyPos), moonDirection);
    vec3 moonContribution = vec3(0.0);
    if (moonDot > 0.99) {
        float moonIntensity = (moonDot - 0.99) / 0.01;
        moonContribution = moonColor * moonIntensity;
    }
    
    FragColor = vec4(skyColor + sunContribution + moonContribution, 1.0);
}

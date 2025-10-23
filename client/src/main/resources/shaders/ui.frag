#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uHasTexture;

void main() {
    vec4 color = vColor;

    if (uHasTexture) {
        vec4 texColor = texture(uTexture, vTexCoord);
        color = texColor * color;
    }

    // Discard fully transparent pixels
    if (color.a < 0.001) {
        discard;
    }

    FragColor = color;
}

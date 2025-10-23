#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uFontAtlas;
uniform vec4 uTextColor;

void main() {
    // Sample the font atlas (single channel - red channel contains alpha)
    float alpha = texture(uFontAtlas, vTexCoord).r;

    // Discard fully transparent pixels
    if (alpha < 0.001) {
        discard;
    }

    FragColor = vec4(uTextColor.rgb, uTextColor.a * alpha);
}

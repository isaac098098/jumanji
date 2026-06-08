#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main() {
    // correct MuPDF pixmap

    vec2 uv = vec2(TexCoord.x, 1.0 - TexCoord.y);
    vec4 tex = texture(ourTexture, uv);
    FragColor = mix(vec4(1.0, 1.0, 1.0, 1.0), tex, tex.a);
}

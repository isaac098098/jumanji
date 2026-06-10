#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat2 rotation_matrix;
uniform mat2 scale_matrix;
uniform mat2 zoom_matrix;
uniform vec2 displacement_vec;

void main() {
    vec2 bPos = rotation_matrix * scale_matrix
              * zoom_matrix * (aPos + displacement_vec);
    gl_Position = vec4(bPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}

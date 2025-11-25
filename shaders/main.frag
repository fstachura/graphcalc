#version 410

in vec3 color;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    out_color = vec4(color, 1.0f);
}

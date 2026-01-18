#version 420

in vec3 color;
smooth sample in vec3 position;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;

void main() {
    if (mod(position.x+0.005, .20f) < 0.01f  || mod(position.z+0.005, .20f) < 0.01f) {
        out_color = vec4(1.0, 1.0, 1.0, 1.0f);
    } else {
        out_color = vec4(0.0, 0.0, 0.0, 0.0f);
    }
}

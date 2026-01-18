#version 410

in vec3 color;
in vec3 position;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;
uniform int lighting;

uniform vec2 center;
uniform vec2 minMax;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

uniform DirectionalLight directional_light;

vec3 calc_directional_light(vec3 base_color, vec3 normal) {
    vec3 light_direction = normalize(-directional_light.direction);
    vec3 view_direction = normalize(camera_position - position);
    vec3 halfway_direction = normalize(light_direction + view_direction);

    float ambient_factor = 0.03f;
    float diffuse_factor = max(dot(normal, light_direction), 0.0f);
    float specular_factor = pow(max(dot(normal, halfway_direction), 0.0f), 64.0f);

    vec3 ambient = ambient_factor * base_color;
    vec3 diffuse = diffuse_factor * directional_light.color * base_color;
    vec3 specular = specular_factor * directional_light.color;

    return ambient + diffuse + specular;
}

void main() {
    out_color = vec4(
        lighting == 1 ? calc_directional_light(color, position) : color,
        1.0f
    );
}

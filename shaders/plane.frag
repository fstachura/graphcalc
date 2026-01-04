#version 410

in vec3 color;
in vec3 position;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 center;

// uniform vec3 camera_position;
// struct DirectionalLight {
//     vec3 direction;
//     vec3 color;
// };
// 
// vec3 normal = vec3(0.0, 0.0, 0.0);
// 
// void calc_directional_light() {
//     vec3 light_direction = normalize(-directional_light.direction);
//     vec3 view_direction = normalize(camera_position - position);
//     vec3 halfway_direction = normalize(light_direction + view_direction);
// 
//     float ambient_factor = 0.03f;
//     float diffuse_factor = max(dot(normal, light_direction), 0.0f);
//     float specular_factor = pow(max(dot(normal, halfway_direction), 0.0f), 64.0f);
// 
//     vec3 ambient = ambient_factor * base_color;
//     vec3 diffuse = diffuse_factor * directional_light.color * base_color;
//     vec3 specular = specular_factor * directional_light.color;
// 
//     return ambient + diffuse + specular;
// }

void main() {
    vec3 c = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0), sin(position.y));

    out_color = vec4(
        // calc_directional_light(),
        c,
        1.0f
    );
}

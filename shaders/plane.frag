#version 410

// based on https://github.com/quazuo/grafika-mimuw/blob/main/07-lighting/shaders/blinn-phong.frag

in vec3 color;
in vec3 position;
in vec3 normal;

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;
uniform int point_light_enabled;
uniform int directional_light_enabled;

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

struct PointLight {
    vec3 position;
    vec3 color;

    float att_constant;
    float att_linear;
    float att_quadratic;
};

uniform PointLight point_light;

vec3 calc_point_light(vec3 base_color, vec3 normal) {
    vec3 light_direction = normalize(point_light.position - position);
    vec3 view_direction = normalize(camera_position - position);
    vec3 halfway_direction = normalize(light_direction + view_direction);

    float distance = length(point_light.position - position);
    float attenuation = 1.0f / (point_light.att_constant + 
                                point_light.att_linear * distance + 
                                point_light.att_quadratic * distance * distance);

    float ambient_factor = 0.03f;
    float diffuse_factor = max(dot(normal, light_direction), 0.0f);
    float specular_factor = pow(max(dot(normal, halfway_direction), 0.0f), 64.0f);

    vec3 ambient = ambient_factor * base_color;
    vec3 diffuse = diffuse_factor * point_light.color * base_color;
    vec3 specular = specular_factor * point_light.color;

    return attenuation * (ambient + diffuse + specular);
}

void main() {
    vec3 result_color = vec3(0.0);

    if (directional_light_enabled == 1) {
        result_color += calc_directional_light(color, normal);
    }

    if (point_light_enabled == 1) {
        result_color += calc_point_light(color, normal);
    }

    if (directional_light_enabled == 0 && point_light_enabled == 0) {
        result_color = color;
    }

    out_color = vec4(result_color, 1.0f);
}

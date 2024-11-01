#version 450

struct Light
{
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    float shininess;
};

layout(std140, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;

    Light light;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    Light light = ubo.light;

    vec3 N = normalize(inNormal);
    vec3 L = normalize(light.position.xyz - inPosition.xyz);
    vec3 V = normalize(-inPosition.xyz);
    vec3 R = reflect(-L, N);

	vec4 ambient = light.ambient;
    float d = length(light.position - inPosition.xyz);
    float denom = light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d;
    vec4 diffuse = max(dot(L, N), 0.0) * light.diffuse / denom;
    vec4 specular = pow(max(dot(R, V), 0.0), 3 * light.shininess) * light.specular / denom;

    outColor = (ambient + diffuse) * texture(texSampler, inTexCoord) + specular;
}
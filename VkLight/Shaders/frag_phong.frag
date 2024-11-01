#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;

    vec3 lightPosition;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    float shininess;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inLightPosition;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightPosition.xyz - inPosition.xyz);
    vec3 V = normalize(-inPosition.xyz);
    vec3 R = reflect(-L, N);

    float shininess = 32;

	vec4 ambient = ubo.ambient;
    float d = length(inLightPosition - inPosition.xyz);
    float denom = ubo.attenuation.x + ubo.attenuation.y * d + ubo.attenuation.z * d * d;
    vec4 diffuse = max(dot(L, N), 0.0) * ubo.diffuse / denom;
    vec4 specular = pow(max(dot(R, V), 0.0), ubo.shininess) * ubo.specular / denom;

    outColor = (ambient + diffuse) * texture(texSampler, inTexCoord) + specular;
}
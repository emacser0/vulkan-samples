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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outLightPosition;

void main()
{
    mat4 modelView = ubo.view * ubo.model;
    outPosition = modelView * vec4(inPosition, 1.0);
    outNormal = vec3(transpose(inverse(modelView)) * vec4(inNormal, 1.0));
    outTexCoord = inTexCoord;
    outLightPosition = ubo.lightPosition;
    gl_Position = ubo.projection * outPosition;
}

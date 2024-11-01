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

struct StorageBufferData
{
    mat4 model;
    mat4 modelView;
    mat4 normalMatrix;
};

layout(std140, binding = 2) readonly buffer StorageBufferObject
{
    StorageBufferData data[];
} storageBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

void main()
{
    Light light = ubo.light;

    StorageBufferData storageBufferData = storageBuffer.data[gl_InstanceIndex];

    mat4 model = storageBufferData.model;
    mat4 modelView = storageBufferData.modelView;
    mat3 normalMatrix = mat3(storageBufferData.normalMatrix);

    outPosition = modelView * vec4(inPosition, 1.0);
    outNormal = normalMatrix * inNormal;
    outTexCoord = inTexCoord;
    gl_Position = ubo.projection * outPosition;
}
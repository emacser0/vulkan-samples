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

layout(location = 0) out vec4 outAmbient;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outSpecular;
layout(location = 3) out vec2 outTexCoord;

void main()
{
    Light light = ubo.light;

    StorageBufferData storageBufferData = storageBuffer.data[gl_InstanceIndex];

    mat4 modelView = storageBufferData.modelView;
    mat3 normalMatrix = mat3(storageBufferData.normalMatrix);

    vec3 normal = normalMatrix * inNormal;
    vec4 position = modelView * vec4(inPosition, 1.0);

    vec3 N = normalize(normal);
    vec3 L = normalize(light.position - position.xyz);
    vec3 V = normalize(-position.xyz);
    vec3 R = reflect(-L, N);

    vec4 ambient = light.ambient;
    float d = length(light.position - position.xyz);
    float denom = light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d;
    vec4 diffuse = max(dot(L, N), 0.0) * light.diffuse / denom;
    vec4 specular = pow(max(dot(R, V), 0.0), light.shininess) * light.specular / denom;

    outAmbient = ambient;
    outDiffuse = diffuse;
    outSpecular = specular;

	gl_Position = ubo.projection * position;
}
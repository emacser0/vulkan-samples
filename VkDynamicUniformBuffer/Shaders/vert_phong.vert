#version 450

layout(binding = 0) uniform UniformBufferObject
{
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

layout(binding = 2) uniform DynamicUniformBufferObject
{
    mat4 model;
} dubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outAmbient;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outSpecular;
layout(location = 3) out vec2 outTexCoord;

void main()
{
    mat4 modelView = ubo.view * dubo.model;
    vec4 position = modelView * vec4(inPosition, 1.0);
    vec4 normal = transpose(inverse(modelView)) * vec4(inNormal, 1.0);

    vec3 N = normalize(normal.xyz);
    vec3 L = normalize(ubo.lightPosition - position.xyz);
    vec3 V = normalize(-position.xyz);
    vec3 R = reflect(-L, N);

    vec4 ambient = ubo.ambient;
    float d = length(ubo.lightPosition - position.xyz);
    float denom = ubo.attenuation.x + ubo.attenuation.y * d + ubo.attenuation.z * d * d;
    vec4 diffuse = max(dot(L, N), 0.0) * ubo.diffuse / denom;
    vec4 specular = pow(max(dot(R, V), 0.0), ubo.shininess) * ubo.specular / denom;

    outAmbient = ambient;
    outDiffuse = diffuse;
    outSpecular = specular;

	gl_Position = ubo.projection * position;
}
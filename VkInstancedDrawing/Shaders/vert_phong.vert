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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in vec4 inModel_0;
layout(location = 4) in vec4 inModel_1;
layout(location = 5) in vec4 inModel_2;
layout(location = 6) in vec4 inModel_3;

layout(location = 7) in vec4 inModelView_0;
layout(location = 8) in vec4 inModelView_1;
layout(location = 9) in vec4 inModelView_2;
layout(location = 10) in vec4 inModelView_3;

layout(location = 11) in vec4 inNormalMatrix_0;
layout(location = 12) in vec4 inNormalMatrix_1;
layout(location = 13) in vec4 inNormalMatrix_2;
layout(location = 14) in vec4 inNormalMatrix_3;

layout(location = 0) out vec4 outAmbient;
layout(location = 1) out vec4 outDiffuse;
layout(location = 2) out vec4 outSpecular;
layout(location = 3) out vec2 outTexCoord;

void main()
{
    Light light = ubo.light;

    mat4 modelView = mat4(inModelView_0, inModelView_1, inModelView_2, inModelView_3);
    mat3 normalMatrix = mat3(mat4(inNormalMatrix_0, inNormalMatrix_1, inNormalMatrix_2, inNormalMatrix_3));

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
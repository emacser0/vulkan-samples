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
layout(location = 3) in vec3 inTangent;

layout(location = 4) in vec4 inModel_0;
layout(location = 5) in vec4 inModel_1;
layout(location = 6) in vec4 inModel_2;
layout(location = 7) in vec4 inModel_3;

layout(location = 8) in vec4 inModelView_0;
layout(location = 9) in vec4 inModelView_1;
layout(location = 10) in vec4 inModelView_2;
layout(location = 11) in vec4 inModelView_3;

layout(location = 12) in vec4 inNormalMatrix_0;
layout(location = 13) in vec4 inNormalMatrix_1;
layout(location = 14) in vec4 inNormalMatrix_2;
layout(location = 15) in vec4 inNormalMatrix_3;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out mat3 outTBN;

void main()
{
    mat4 modelView = mat4(inModelView_0, inModelView_1, inModelView_2, inModelView_3);
    mat3 normalMatrix = mat3(mat4(inNormalMatrix_0, inNormalMatrix_1, inNormalMatrix_2, inNormalMatrix_3));

    outPosition = modelView * vec4(inPosition, 1.0);
    outNormal = normalize(normalMatrix * inNormal);
    outTexCoord = inTexCoord;

    vec3 tangent = normalize(normalMatrix * inTangent);
    vec3 bitangent = normalize(normalMatrix * cross(outNormal, tangent));
    outTBN = mat3(tangent, bitangent, outNormal);

    gl_Position = ubo.projection * outPosition;
}
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

layout(std140, binding = 0) uniform TransformBufferObject
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} transformBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 4) in mat4 inModel;
layout(location = 8) in mat4 inModelView;
layout(location = 12) in mat4 inNormalMatrix;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outTangent;
layout(location = 2) out vec3 outBitangent;

void main()
{
    mat3 normalMatrix = mat3(inNormalMatrix);

    vec4 position = inModelView * vec4(inPosition, 1.0);
    gl_Position = transformBuffer.projection * position;

    outNormal = normalize(vec3(transformBuffer.projection * vec4(normalMatrix * inNormal, 0.0)));
    outTangent = normalize(vec3(transformBuffer.projection * vec4(normalMatrix * inTangent, 0.0)));
    outBitangent = normalize(cross(outNormal, outTangent));
}
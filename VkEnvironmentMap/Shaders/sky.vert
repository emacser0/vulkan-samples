#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
    mat4 cameraRotation;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outTexCoord;

void main()
{
    outTexCoord = inPosition;
    gl_Position = ubo.projection * ubo.cameraRotation * vec4(inPosition, 1.0);
}
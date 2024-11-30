#version 450

layout(std140, binding = 0) uniform TransformBuffer
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

layout(location = 0) out vec4 outPosition;

void main()
{
    gl_Position = transformBuffer.projection * inModelView * vec4(inPosition, 1.0);
}
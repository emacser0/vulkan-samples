#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inAmbient;
layout(location = 1) in vec4 inDiffuse;
layout(location = 2) in vec4 inSpecular;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = (inAmbient + inDiffuse) * texture(texSampler, inTexCoord) + inSpecular;
}
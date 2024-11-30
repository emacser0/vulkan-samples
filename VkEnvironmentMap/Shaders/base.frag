#version 450

struct PointLight
{
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    float shininess;
};

struct DirectionalLight
{
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    float shininess;
};

layout(std140, binding = 0) uniform TransformBuffer
{
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
} transformBuffer;

layout(std140, binding = 1) uniform LightBuffer
{
    uint numPointLights;
    PointLight pointLights[16];

    uint numDirectionalLights;
    DirectionalLight directionalLights[16];
} lightBuffer;

layout(std140, binding = 2) uniform DebugBuffer
{
    bool bAttenuation;
    bool bGammaCorrection;
    bool bToneMapping;
} debugBuffer;

layout(binding = 3) uniform sampler2D baseColorSampler;
layout(binding = 4) uniform sampler2D normalSampler;
layout(binding = 5) uniform samplerCube cubemapSampler;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

vec4 hdrToneMapping(vec4 inColor)
{
    vec3 outColor = vec3(inColor);
    outColor = outColor / (outColor + vec3(1.0));

    return vec4(outColor, inColor.a);
}

vec4 gammaCorrection(vec4 inColor)
{
    vec3 outColor = vec3(inColor);
    outColor = pow(outColor, vec3(1.0 / 2.2));

    return vec4(outColor, inColor.a);
}

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(transformBuffer.cameraPosition - inPosition.xyz);

    vec3 R = reflect(-V, N);

    vec4 baseColor = texture(cubemapSampler, R);
    outColor = baseColor;

    if (debugBuffer.bGammaCorrection)
    {
		outColor = gammaCorrection(outColor);
    }

    if (debugBuffer.bToneMapping)
    {
		outColor = hdrToneMapping(outColor);
    }
}
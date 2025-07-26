#version 450

struct PointLight
{
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    float shininess;
    mat4 lightSpaceMatrix;
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

layout(std140, binding = 1) uniform LightBuffer
{
    uint numPointLights;
    PointLight pointLights[16];

    uint numDirectionalLights;
    DirectionalLight directionalLights[16];
} lightBuffer;

layout(std140, binding = 2) uniform MaterialBuffer
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} materialBuffer;

layout(std140, binding = 3) uniform DebugBuffer
{
    bool bAttenuation;
    bool bGammaCorrection;
    bool bToneMapping;
} debugBuffer;

layout(binding = 4) uniform sampler2D baseColorSampler;
layout(binding = 5) uniform sampler2D normalSampler;
layout(binding = 6) uniform sampler2D shadowSampler;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat3 inTBN;

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
    outColor = pow(outColor, vec3(1.0 / 1.2));

    return vec4(outColor, inColor.a);
}

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(-inPosition.xyz);

    vec3 tangentNormal = normalize(texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0);
    N = normalize(inTBN * tangentNormal);

    vec4 baseColor = texture(baseColorSampler, inTexCoord);

    vec4 ambient = vec4(0.0);
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    for (int i = 0; i < lightBuffer.numPointLights; ++i)
    {
		PointLight light = lightBuffer.pointLights[i];

		vec3 L = normalize(light.position.xyz - inPosition.xyz);
		vec3 H = normalize(L + V);
		
		float d = length(light.position - inPosition.xyz);
		float denom = light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d;

        if (!debugBuffer.bAttenuation)
        {
            denom = 1.0;
        }

        ambient += materialBuffer.ambient * light.ambient;
		diffuse += materialBuffer.diffuse * light.diffuse * max(dot(L, N), 0.0) / denom;
		specular += materialBuffer.specular * light.specular * pow(max(dot(N, H), 0.0), 3 * light.shininess) / denom;
    }

    for (int i = 0; i < lightBuffer.numDirectionalLights; ++i)
    {
        DirectionalLight light = lightBuffer.directionalLights[i];

        vec3 L = normalize(light.direction);
        vec3 H = normalize(L + V);

        ambient += materialBuffer.ambient * light.ambient;
		diffuse += materialBuffer.diffuse * light.diffuse * max(dot(L, N), 0.0);
		specular += materialBuffer.specular * light.specular * pow(max(dot(N, H), 0.0), 3 * light.shininess);
    }

    outColor = (ambient + diffuse + specular) * texture(baseColorSampler, inTexCoord);

    if (debugBuffer.bGammaCorrection)
    {
		outColor = gammaCorrection(outColor);
    }

    if (debugBuffer.bToneMapping)
    {
		outColor = hdrToneMapping(outColor);
    }
}
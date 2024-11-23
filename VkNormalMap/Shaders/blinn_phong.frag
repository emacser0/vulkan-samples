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

layout(std140, binding = 1) uniform LightBuffer
{
    uint numPointLights;
    PointLight pointLights[16];

    uint numDirectionalLights;
    DirectionalLight directionalLights[16];
} lightBuffer;

layout(binding = 2) uniform sampler2D baseColorSampler;
layout(binding = 3) uniform sampler2D normalSampler;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat3 inTBN;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 N = normalize(inNormal);
    vec3 V = normalize(-inPosition.xyz);

    vec3 tangentNormal = normalize(texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0);
    N = normalize(inTBN * tangentNormal);

    vec4 baseColor = texture(baseColorSampler, inTexCoord);

    vec4 ambient = lightBuffer.pointLights[0].ambient;
    vec4 diffuse = vec4(0.0);
    vec4 specular = vec4(0.0);

    for (int i = 0; i < lightBuffer.numPointLights; ++i)
    {
		PointLight light = lightBuffer.pointLights[i];

		vec3 L = normalize(light.position.xyz - inPosition.xyz);
		vec3 H = normalize(L + V);
		
		float d = length(light.position - inPosition.xyz);
		float denom = light.attenuation.x + light.attenuation.y * d + light.attenuation.z * d * d;

		diffuse += max(dot(L, N), 0.0) * light.diffuse / denom;
		specular += pow(max(dot(N, H), 0.0), 3 * light.shininess) * light.specular / denom;
    }

    for (int i = 0; i < lightBuffer.numDirectionalLights; ++i)
    {
        DirectionalLight light = lightBuffer.directionalLights[i];

        vec3 L = normalize(light.direction);
        vec3 H = normalize(L + V);

		vec4 ambient = light.ambient;
		diffuse += max(dot(L, N), 0.0) * light.diffuse;
		specular += pow(max(dot(N, H), 0.0), 3 * light.shininess) * light.specular;
    }

    outColor = (ambient + diffuse) * texture(baseColorSampler, inTexCoord) + specular;
}
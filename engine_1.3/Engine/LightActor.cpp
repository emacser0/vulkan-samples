#include "LightActor.h"
#include "Engine.h"
#include "Texture.h"

#include "Config.h"
#include "AssetManager.h"

#include "VulkanContext.h"
#include "VulkanModel.h"
#include "VulkanMesh.h"

ALightActor::ALightActor()
	: AActor()
	, Ambient(0.0f, 0.0f, 0.0f, 1.0f)
	, Diffuse(0.0f, 0.0f, 0.0f, 1.0f)
	, Specular(0.0f, 0.0f, 0.0f, 1.0f)
	, Attenuation(1.0f, 0.0f, 0.0f, 1.0f)
	, Shininess(0.0f)
{
}

ALightActor::~ALightActor()
{
}


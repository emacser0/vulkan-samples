#include "VulkanScene.h"
#include "VulkanContext.h"
#include "VulkanModel.h"

FVulkanScene::FVulkanScene(FVulkanContext* InContext)
	: FVulkanObject(InContext)
{

}

FVulkanScene::~FVulkanScene()
{
}

void FVulkanScene::AddModel(FVulkanModel* InModel)
{
	if (InModel == nullptr)
	{
		return;
	}

	Models.push_back(InModel);
}

void FVulkanScene::RemoveModel(FVulkanModel* InModel)
{
	if (InModel == nullptr)
	{
		return;
	}

	for (auto Itr = Models.begin(); Itr != Models.end(); ++Itr)
	{
		if (*Itr == InModel)
		{
			Context->DestroyObject(*Itr);
			Models.erase(Itr);
			break;
		}
	}
}

void FVulkanScene::ClearModels()
{
	for (FVulkanModel* Model : Models)
	{
		Context->DestroyObject(Model);
	}

	Models.clear();
}


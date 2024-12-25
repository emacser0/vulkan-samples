#include "VulkanScene.h"
#include "VulkanModel.h"

FVulkanScene::FVulkanScene(FVulkanContext* InContext)
	: FVulkanObject(InContext)
{

}

FVulkanScene::~FVulkanScene()
{
	for (FVulkanModel* Model : Models)
	{
		delete Model;
	}
}

void FVulkanScene::AddModel(FVulkanModel* InModel)
{
	Models.push_back(InModel);
}

void FVulkanScene::RemoveModel(FVulkanModel* InModel)
{
	std::remove_if(Models.begin(), Models.end(), [InModel](FVulkanModel* Model)
	{
		return Model == InModel;
	});
}


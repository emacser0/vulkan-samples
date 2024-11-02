#include "AssetManager.h"
#include "Asset.h"

#include <cassert>

FAssetManager* FAssetManager::Instance;

void FAssetManager::Startup()
{
	Instance = new FAssetManager();
}

void FAssetManager::Shutdown()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

void FAssetManager::DestroyAsset(FAsset* InAsset)
{
	assert(Instance != nullptr);

	if (InAsset == nullptr)
	{
		return;
	}

	for (int Idx = 0; Idx < Instance->LiveAssets.size(); ++Idx)
	{
		if (InAsset == Instance->LiveAssets[Idx])
		{
			Instance->LiveAssets[Idx] = nullptr;
			break;
		}
	}
}

FAssetManager::FAssetManager()
{

}

FAssetManager::~FAssetManager()
{
	for (FAsset* Asset : LiveAssets)
	{
		if (Asset != nullptr)
		{
			delete Asset;
		}
	}
}
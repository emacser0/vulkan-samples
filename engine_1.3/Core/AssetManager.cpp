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

UAsset* FAssetManager::FindAsset(const std::string& InAssetName)
{
	if (Instance == nullptr)
	{
		return nullptr;
	}

	std::unordered_map<std::string, UAsset*> LiveAssets = Instance->LiveAssets;
	if (auto Itr = LiveAssets.find(InAssetName); Itr != LiveAssets.end())
	{
		return Itr->second;
	}

	return nullptr;
}

void FAssetManager::DestroyAsset(const std::string& InAssetName)
{
	assert(Instance != nullptr);

	std::unordered_map<std::string, UAsset*>& LiveAssets = Instance->LiveAssets;
	if (auto Itr = LiveAssets.find(InAssetName); Itr != LiveAssets.end())
	{
		LiveAssets.erase(Itr);
		delete Itr->second;
	}
}

void FAssetManager::DestroyAsset(UAsset* InAsset)
{
	if (InAsset == nullptr)
	{
		return;
	}
	
	DestroyAsset(InAsset->GetName());
}

FAssetManager::FAssetManager()
{

}

FAssetManager::~FAssetManager()
{
	for (const auto& Pair : LiveAssets)
	{
		if (Pair.second != nullptr)
		{
			delete Pair.second;
		}
	}
}
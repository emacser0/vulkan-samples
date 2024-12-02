#pragma once

#include <unordered_map>
#include <string>

#include "Utils.h"

class UAsset;

class FAssetManager
{
public:
	static void Startup();
	static void Shutdown();

	template <typename T = UAsset>
	static T* CreateAsset(const std::string& InAssetName)
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		std::unordered_map<std::string, UAsset*>& LiveAssets = Instance->LiveAssets;

		auto Itr = LiveAssets.find(InAssetName);
		if (Itr != LiveAssets.end())
		{
			return Cast<T>(Itr->second);
		}

		T* NewAsset = T::StaticCreateObject();
		NewAsset->SetName(InAssetName);

		LiveAssets[InAssetName] = NewAsset;

		return NewAsset;
	}

	static UAsset* FindAsset(const std::string& InAssetName);
	static void DestroyAsset(const std::string& InAssetName);
	static void DestroyAsset(UAsset* InAsset);

private:
	static FAssetManager* Instance;

private:
	FAssetManager();

public:
	virtual ~FAssetManager();

private:
	std::unordered_map<std::string, class UAsset*> LiveAssets;
};

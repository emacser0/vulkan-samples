#pragma once

#include <vector>

class FAsset;

class FAssetManager
{
public:
	static void Startup();
	static void Shutdown();

	template <typename T = FAsset>
	static T* CreateAsset()
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		T* NewAsset = new T();
		Instance->LiveAssets.push_back(NewAsset);

		return NewAsset;
	}

	void DestroyAsset(FAsset* InAsset);

private:
	static FAssetManager* Instance;

private:
	FAssetManager();

public:
	virtual ~FAssetManager();

private:
	std::vector<class FAsset*> LiveAssets;
};

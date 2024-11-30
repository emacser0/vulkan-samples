#pragma once

#include "Object.h"

#include <string>

class UAsset : public UObject
{
public:
	DECLARE_OBJECT_BODY(UAsset, UObject);

	UAsset();
	virtual ~UAsset();

	std::string GetName() const { return Name; }
	void SetName(const std::string& InName) { Name = InName; }

private:
	std::string Name;
};

#pragma once

#include "Asset.h"
#include "Vertex.h"

#include <string>

class FSkyMesh : public FAsset
{
public:
	FSkyMesh();
	virtual ~FSkyMesh();

	virtual bool Load(const std::string& In)
};

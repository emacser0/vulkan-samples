#pragma once

#include <string>
#include <vector>

#include "Texture.h"

bool LoadModel(const std::string& InFilename, std::vector<struct FVertex>& OutVertices, std::vector<uint16_t>& OutIndices);
bool LoadTexture(const std::string& InFilename, FTexture& OutTexture);

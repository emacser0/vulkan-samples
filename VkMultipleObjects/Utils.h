#pragma once

#include <string>
#include <vector>

#include "stb_image.h"

struct FVertex;

bool ReadFile(const std::string& InFilename, std::vector<char>& OutBytes);
bool LoadModel(const std::string& InFilename, std::vector<FVertex>& OutVertices, std::vector<uint32_t>& OutIndices);

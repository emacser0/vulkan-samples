#pragma once

#include <string>
#include <vector>

bool LoadModel(const std::string& InFilename, std::vector<struct FVertex>& OutVertices, std::vector<uint32_t>& OutIndices);

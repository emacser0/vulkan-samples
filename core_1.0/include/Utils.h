#pragma once

#include <string>
#include <vector>

#include "stb_image.h"

bool ReadFile(const std::string& InFilename, std::vector<char>& OutBytes);

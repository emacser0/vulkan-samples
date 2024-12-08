#include "Utils.h"

#include <iostream>
#include <fstream>

bool ReadFile(const std::string& InFilename, std::vector<char>& OutBytes)
{
	std::ifstream File(InFilename, std::ios::ate | std::ios::binary);
	if (File.is_open() == false)
	{
		return false;
	}

	size_t FileSize = (size_t)File.tellg();
	OutBytes.resize(FileSize);

	File.seekg(0);
	File.read(OutBytes.data(), FileSize);

	File.close();

	return true;
}


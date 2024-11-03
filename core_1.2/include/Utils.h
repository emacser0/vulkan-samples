#pragma once

#include <string>
#include <vector>
#include <type_traits>

#include "stb_image.h"

bool ReadFile(const std::string& InFilename, std::vector<char>& OutBytes);

template <typename T>
inline void CombineHash(std::size_t& InSeed, const T& V)
{
	std::hash<T> Hasher;
	InSeed ^= Hasher(V) + 0x9e3779b6 + (InSeed << 6) + (InSeed >> 2);
}

template <typename To, typename From>
To* Cast(From* Ptr)
{
	if constexpr (std::is_base_of_v<From, To> || std::is_base_of_v<To, From>)
	{
		return dynamic_cast<To*>(Ptr);
	}

	return nullptr;
}

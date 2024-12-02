#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

#include "glm/glm.hpp"

enum class EConfigValueType
{
	Int,
	Int64,
	Float,
	Double,
	Bool,
	String,
	Array,
	Vector2,
	Vector3,
	Vector4,
	None
};

struct FConfigValue
{
public:
	FConfigValue();
	FConfigValue(EConfigValueType InType);

	EConfigValueType GetType() const { return Type; }

	void Get(int32_t& OutValue) const;
	void Get(int64_t& OutValue) const;
	void Get(float& OutValue) const;
	void Get(double& OutValue) const;
	void Get(bool& OutValue) const;
	void Get(std::string& OutValue) const;
	void Get(std::vector<std::string>& OutValue) const;
	void Get(glm::vec2& OutValue) const;
	void Get(glm::vec3& OutValue) const;
	void Get(glm::vec4& OutValue) const;

	void Set(int32_t InValue);
	void Set(int64_t InValue);
	void Set(float InValue);
	void Set(double InValue);
	void Set(bool InValue);
	void Set(const std::string& InValue);
	void Set(const char* InValue);
	void Set(const std::vector<std::string>& InValue);
	void Set(const glm::vec2& InValue);
	void Set(const glm::vec3& InValue);
	void Set(const glm::vec4& InValue);

private:
	EConfigValueType Type;

	int32_t IntValue;
	int64_t Int64Value;
	float FloatValue;
	double DoubleValue;
	bool BoolValue;
	std::string StringValue;
	std::vector<std::string> ArrayValue;
	glm::vec2 Vector2Value;
	glm::vec3 Vector3Value;
	glm::vec4 Vector4Value;
};

class FConfig
{
public:
	static void Startup();
	static void Shutdown();

	FConfig();
	virtual ~FConfig();

	template <typename T>
	void Get(const std::string& InKey, T& OutValue) const;

	template <typename T>
	void Set(const std::string& InKey, const T& InValue);

	template <typename T>
	void Set(const char* InKey, const T& InValue);

private:
	std::map<std::string, FConfigValue> ConfigMap;
};

template <typename T>
void FConfig::Get(const std::string& InKey, T& OutValue) const
{	
	auto Iter = ConfigMap.find(InKey);
	if (Iter == ConfigMap.end())
	{
		return;
	}

	const FConfigValue& Value = Iter->second;
	Value.Get(OutValue);
}

template <typename T>
void FConfig::Set(const std::string& InKey, const T& InValue)
{
	FConfigValue NewValue;
	NewValue.Set(InValue);

	ConfigMap[InKey] = NewValue;
}

template <typename T>
void FConfig::Set(const char* InKey, const T& InValue)
{
	FConfigValue NewValue;
	NewValue.Set(InValue);

	ConfigMap[InKey] = NewValue;
}

extern FConfig* GConfig;


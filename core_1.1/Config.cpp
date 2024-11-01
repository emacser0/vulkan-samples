#include "Config.h"

FConfig* GConfig;

FConfigValue::FConfigValue()
	: Type(EConfigValueType::None)
{
}

FConfigValue::FConfigValue(EConfigValueType InType)
	: Type(InType)
{

}

void FConfigValue::Get(int32_t& OutValue) const
{
	assert(Type == EConfigValueType::Int);
	OutValue = IntValue;
}

void FConfigValue::Get(int64_t& OutValue) const
{
	assert(Type == EConfigValueType::Int64);
	OutValue = Int64Value;
}

void FConfigValue::Get(float& OutValue) const
{
	assert(Type == EConfigValueType::Float);
	OutValue = FloatValue;
}

void FConfigValue::Get(double& OutValue) const
{
	assert(Type == EConfigValueType::Double);
	OutValue = DoubleValue;
}

void FConfigValue::Get(bool& OutValue) const
{
	assert(Type == EConfigValueType::Bool);
	OutValue = BoolValue;
}

void FConfigValue::Get(std::string& OutValue) const
{
	assert(Type == EConfigValueType::String);
	OutValue = StringValue;
}

void FConfigValue::Get(std::vector<std::string>& OutValue) const
{
	assert(Type == EConfigValueType::Array);
	OutValue = ArrayValue;
}

void FConfigValue::Get(glm::vec2& OutValue) const
{
	assert(Type == EConfigValueType::Vector2);
	OutValue = Vector2Value;
}

void FConfigValue::Get(glm::vec3& OutValue) const
{
	assert(Type == EConfigValueType::Vector3);
	OutValue = Vector3Value;
}

void FConfigValue::Get(glm::vec4& OutValue) const
{
	assert(Type == EConfigValueType::Vector4);
	OutValue = Vector4Value;
}

void FConfigValue::Set(int32_t InValue)
{
	Type = EConfigValueType::Int;
	IntValue = InValue;
}

void FConfigValue::Set(int64_t InValue)
{
	Type = EConfigValueType::Int64;
	Int64Value = InValue;
}

void FConfigValue::Set(float InValue)
{
	Type = EConfigValueType::Float;
	FloatValue = InValue;
}

void FConfigValue::Set(double InValue)
{
	Type = EConfigValueType::Double;
	DoubleValue = InValue;
}

void FConfigValue::Set(bool InValue)
{
	Type = EConfigValueType::Bool;
	BoolValue = InValue;
}

void FConfigValue::Set(const std::string& InValue)
{
	Type = EConfigValueType::String;
	StringValue = InValue;
}

void FConfigValue::Set(const char* InValue)
{
	Type = EConfigValueType::String;
	StringValue = InValue;
}

void FConfigValue::Set(const std::vector<std::string>& InValue)
{
	Type = EConfigValueType::Array;
	ArrayValue = InValue;
}

void FConfigValue::Set(const glm::vec2& InValue)
{
	Type = EConfigValueType::Vector2;
	Vector2Value = InValue;
}

void FConfigValue::Set(const glm::vec3& InValue)
{
	Type = EConfigValueType::Vector3;
	Vector3Value = InValue;

}

void FConfigValue::Set(const glm::vec4& InValue)
{
	Type = EConfigValueType::Vector4;
	Vector4Value = InValue;
}

void FConfig::Startup()
{
	GConfig = new FConfig();
}

void FConfig::Shutdown()
{
	delete GConfig;
	GConfig = nullptr;
}

FConfig::FConfig()
{

}

FConfig::~FConfig()
{

}

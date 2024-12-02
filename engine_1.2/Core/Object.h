#pragma once

#include <string>

#define DECLARE_OBJECT_BODY_INTERNAL(ClassName) \
	static std::string StaticTypeId() { return #ClassName; } \
	static ClassName* StaticCreateObject() \
	{ \
		ClassName* NewObject = new ClassName(); \
		NewObject->TypeId = ClassName::StaticTypeId(); \
		return NewObject; \
	} \

#define DECLARE_OBJECT_BODY(ClassName, ParentClassName) \
	DECLARE_OBJECT_BODY_INTERNAL(ClassName); \
	using Super = ParentClassName; \

class UObject
{
public:
	DECLARE_OBJECT_BODY_INTERNAL(UObject)

	UObject() { }
	virtual ~UObject() { }

	std::string GetTypeId() const { return TypeId; }

protected:
	std::string TypeId;
};

template <typename To = UObject, typename From = UObject>
To* Cast(From* Ptr)
{
	if (Ptr == nullptr)
	{
		return nullptr;
	}

	if (Ptr->GetTypeId() != To::StaticTypeId())
	{
		return nullptr;
	}

	return dynamic_cast<To*>(Ptr);
}

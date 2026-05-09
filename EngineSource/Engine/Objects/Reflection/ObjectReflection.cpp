#include "ObjectReflection.h"
#include <iostream>

using namespace engine;

std::unordered_map<ObjectTypeID, Reflection::ObjectInfo> Reflection::ObjectTypes;

std::vector<Reflection::ObjectInfo>* Reflection::MacroTypes = nullptr;

ObjectTypeID engine::Reflection::RegisterObjectMacro(string Name, std::function<ReflectionObject* ()> NewFunc,
	string Category, int32 Super)
{
	ObjectTypeID ID = str::Hash(Category + "/" + Name);

	if (!MacroTypes)
	{
		MacroTypes = new std::vector<ObjectInfo>();
	}

	if (Super == str::Hash("ReflectionObject"))
	{
		Super = 0;
	}

	MacroTypes->push_back(ObjectInfo{
		.TypeID = ID,
		.Name = Name,
		.Path = Category,
		.SuperClass = Super,
		.CreateInstance = NewFunc,
		});

	return ID;
}

engine::ObjectTypeID engine::Reflection::RegisterObject(string Name, std::function<ReflectionObject* ()> NewFunc,
	int32 Super, string Category)
{
	ObjectTypeID ID = str::Hash(Category + "/" + Name);

	if (!MacroTypes)
	{
		MacroTypes = new std::vector<ObjectInfo>();
	}

	ObjectTypes[ID] = ObjectInfo{
		.TypeID = ID,
		.Name = Name,
		.Path = Category,
		.SuperClass = Super,
		.CreateInstance = NewFunc,
	};

	return ID;
}

void engine::Reflection::UnRegisterObject(ObjectTypeID Id)
{
	ObjectTypes.erase(Id);
}

void engine::Reflection::Init()
{
	if (MacroTypes)
	{
		for (auto& i : *MacroTypes)
		{
			ObjectTypes.insert({ i.TypeID, i });
		}
	}
}

bool engine::Reflection::ObjectInfo::IsSubclassOf(ObjectTypeID Class) const
{
	if (Class == TypeID)
	{
		return true;
	}

	if (Class == 0)
	{
		return true;
	}

	if (Class == SuperClass)
	{
		return true;
	}

	if (SuperClass)
	{
		return Reflection::ObjectTypes.at(SuperClass).IsSubclassOf(Class);
	}
	return false;
}

#include "ObjectReflection.h"
#include <iostream>

std::unordered_map<engine::ObjectTypeID, engine::Reflection::ObjectInfo> engine::Reflection::ObjectTypes;
std::vector<engine::Reflection::ObjectInfo>* engine::Reflection::MacroTypes = nullptr;

engine::ObjectTypeID engine::Reflection::RegisterObjectMacro(string Name, std::function<SceneObject* ()> NewFunc, string Category)
{
	ObjectTypeID ID = str::Hash(Name + Category);

	if (!MacroTypes)
	{
		MacroTypes = new std::vector<ObjectInfo>();
	}

	MacroTypes->push_back(ObjectInfo{
		.TypeID = ID,
		.Name = Name,
		.Path = Category,
		.CreateInstance = NewFunc,
		});

	return ID;
}

engine::ObjectTypeID engine::Reflection::RegisterObject(string Name, std::function<SceneObject* ()> NewFunc, string Category)
{
	ObjectTypeID ID = str::Hash(Name + Category);

	if (!MacroTypes)
	{
		MacroTypes = new std::vector<ObjectInfo>();
	}

	ObjectTypes[ID] = ObjectInfo{
		.TypeID = ID,
		.Name = Name,
		.Path = Category,
		.CreateInstance = NewFunc,
		};

	return ID;
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

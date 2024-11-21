#include "ObjectPropery.h"
#include <Engine/Objects/SceneObject.h>

using namespace engine;

engine::ObjProperty<string>::ObjProperty(string Name, string Value, SceneObject* Obj)
{
	this->Name = Name;
	this->Value = Value;
	Obj->Properties.push_back(this);
}

engine::ObjProperty<AssetRef>::ObjProperty(string Name, AssetRef Value, SceneObject* Obj)
{
	this->Name = Name;
	this->Value = Value;
	Obj->Properties.push_back(this);
}

SerializedValue engine::ObjProperty<string>::Serialize()
{
	return Value;
}

void engine::ObjProperty<string>::DeSerialize(SerializedValue* From)
{
	Value = From->GetString();
}

SerializedValue engine::ObjProperty<AssetRef>::Serialize()
{
	return Value.FilePath;
}

void engine::ObjProperty<AssetRef>::DeSerialize(SerializedValue* From)
{
	Value.FilePath = From->GetString();
}

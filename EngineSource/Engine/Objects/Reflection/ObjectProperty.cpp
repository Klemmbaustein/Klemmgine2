#include "ObjectPropery.h"
#include "ObjectPropery.h"
#include <Engine/Objects/SceneObject.h>

using namespace engine;

engine::ObjProperty<string>::ObjProperty(string Name, string Value, SceneObject* Obj)
{
	this->Type = PropertyType::String;
	this->Name = Name;
	this->Value = Value;
	RegisterSelf(Obj);
}

engine::ObjProperty<AssetRef>::ObjProperty(string Name, AssetRef Value, SceneObject* Obj)
{
	this->Type = PropertyType::AssetRef;
	this->Name = Name;
	this->Value = Value;
	RegisterSelf(Obj);
}

engine::ObjProperty<float>::ObjProperty(string Name, float Value, SceneObject* Obj)
{
	this->Type = PropertyType::Float;
	this->Name = Name;
	this->Value = Value;
	RegisterSelf(Obj);
}

engine::ObjProperty<bool>::ObjProperty(string Name, bool Value, SceneObject* Obj)
{
	this->Type = PropertyType::Bool;
	this->Name = Name;
	this->Value = Value;
	RegisterSelf(Obj);
}

SerializedValue engine::ObjProperty<string>::Serialize()
{
	return Value;
}

void engine::ObjProperty<string>::DeSerialize(SerializedValue* From)
{
	Value = From->GetString();
}

SerializedValue engine::ObjProperty<float>::Serialize()
{
	return Value;
}

void engine::ObjProperty<float>::DeSerialize(SerializedValue* From)
{
	Value = From->GetFloat();
}

SerializedValue engine::ObjProperty<AssetRef>::Serialize()
{
	return Value.FilePath;
}

SerializedValue engine::ObjProperty<bool>::Serialize()
{
	return Value;
}

void engine::ObjProperty<bool>::DeSerialize(SerializedValue* From)
{
	Value = From->GetBool();
}

void engine::ObjProperty<AssetRef>::DeSerialize(SerializedValue* From)
{
	Value = AssetRef::Convert(From->GetString());
}

void engine::ObjPropertyBase::RegisterSelf(SceneObject* Parent)
{
	Parent->Properties.push_back(this);
}
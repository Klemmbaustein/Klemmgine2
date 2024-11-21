#include "SceneObject.h"
#include <iostream>
#include <Engine/File/ModelData.h>
using namespace engine;

SerializedValue engine::SceneObject::Serialize()
{
	SerializedValue SerializedProperties = std::vector<SerializedValue>();

	for (auto& i : this->Properties)
	{
		SerializedProperties.Append(SerializedValue({
			SerializedData("name", i->Name),
			SerializedData("val", i->Serialize())
			}));
	}

	return SerializedValue({
		SerializedData("position", Position),
		SerializedData("name", Name),
		SerializedData("typeId", int32(TypeID)),
		SerializedData("properties", SerializedProperties)
		});
}

void engine::SceneObject::DeSerialize(SerializedValue* From)
{
	Position = From->At("position").GetVector3();
	Name = From->At("name").GetString();
	TypeID = ObjectTypeID(From->At("typeId").GetInt());

	auto& Array = From->At("properties").GetArray();
	for (SerializedValue& i : Array)
	{
		string Name = i.At("name").GetString();
		SerializedValue& Value = i.At("val");

		for (auto& prop : Properties)
		{
			if (prop->Name == Name)
			{
				prop->DeSerialize(&Value);
				ObjProperty<AssetRef>* Ref = dynamic_cast<ObjProperty<AssetRef>*>(prop);
				if (Ref)
				{
					GraphicsModel::RegisterModel(Ref->Value.FilePath);
				}
			}
		}
	}
}

Scene* engine::SceneObject::GetScene()
{
	return OriginScene;
}

void engine::SceneObject::Draw(graphics::Camera* Cam)
{
}

void engine::SceneObject::Begin()
{
}

void engine::SceneObject::InitObj(Scene* Scn, bool CallBegin, int32 TypeID)
{
	OriginScene = Scn;
	this->TypeID = TypeID;
	if (CallBegin)
		Begin();
}

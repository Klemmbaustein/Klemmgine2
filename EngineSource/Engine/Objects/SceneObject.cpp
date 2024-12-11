#include "SceneObject.h"
#include <iostream>
#include <Engine/File/ModelData.h>
#include <Engine/Scene.h>
using namespace engine;

SerializedValue engine::SceneObject::Serialize()
{
	SerializedValue SerializedProperties = std::vector<SerializedData>();

	for (auto& i : this->Properties)
	{
		SerializedProperties.Append({
			SerializedData(i->Name, i->Serialize()),
			});
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

	if (From->Contains("properties"))
	{
		auto& Array = From->At("properties").GetObject();
		for (SerializedData& i : Array)
		{
			const string& Name = i.Name;
			SerializedValue& Value = i.Value;

			for (auto& prop : Properties)
			{
				if (prop->Name == Name)
				{
					prop->DeSerialize(&Value);
					ObjProperty<AssetRef>* Ref = dynamic_cast<ObjProperty<AssetRef>*>(prop);
					if (Ref)
					{
						GraphicsModel::RegisterModel(AssetRef(Ref->Value.FilePath));
						if (OriginScene)
						{
							OriginScene->ReferencedAssets.push_back(Ref->Value);
						}
					}
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

void engine::SceneObject::Destroy()
{
}

void engine::SceneObject::InitObj(Scene* Scn, bool CallBegin, int32 TypeID)
{
	OriginScene = Scn;
	this->TypeID = TypeID;
	if (CallBegin)
		Begin();
}

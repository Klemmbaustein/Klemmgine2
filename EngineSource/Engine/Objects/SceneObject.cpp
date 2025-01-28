#include "SceneObject.h"
#include <Engine/Scene.h>
#include <Engine/Error/EngineAssert.h>
#include <Engine/Log.h>
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
		SerializedData("rotation", Rotation.EulerVector()),
		SerializedData("scale", Scale),
		SerializedData("name", Name),
		SerializedData("typeId", int32(TypeID)),
		SerializedData("properties", SerializedProperties)
		});
}

void engine::SceneObject::DeSerialize(SerializedValue* From)
{
	Position = From->At("position").GetVector3();

	Rotation = From->At("rotation").GetVector3();

	Scale = From->At("scale").GetVector3();

	Name = From->At("name").GetString();
	TypeID = ObjectTypeID(From->At("typeId").GetInt());

	std::set<string> PreLoaded;
	if (From->Contains("properties"))
	{
		auto& Array = From->At("properties").GetObject();
		for (SerializedData& i : Array)
		{
			const string& Name = i.Name;
			SerializedValue& Value = i.Value;

			for (auto& prop : Properties)
			{
				if (prop->Name != Name)
				{
					continue;
				}
				prop->DeSerialize(&Value);
				if (BeginCalled && prop->OnChanged)
					prop->OnChanged();
				ObjProperty<AssetRef>* Ref = dynamic_cast<ObjProperty<AssetRef>*>(prop);
				if (Ref && OriginScene)
				{
					PreLoaded.insert(Name);
					OriginScene->PreLoadAsset(Ref->Value);
				}
			}
		}
	}

	if (OriginScene)
	{
		for (auto& prop : Properties)
		{
			ObjProperty<AssetRef>* Ref = dynamic_cast<ObjProperty<AssetRef>*>(prop);

			if (!PreLoaded.contains(prop->Name))
			{
				OriginScene->PreLoadAsset(Ref->Value);
			}
		}
	}
}

void engine::SceneObject::Attach(ObjectComponent* Component)
{
	ENGINE_ASSERT(!Component->ParentComponent && !Component->ParentObject);
	Component->ParentObject = this;
	Component->UpdateTransform();
	Component->OnAttached();
	ChildComponents.push_back(Component);
}

void engine::SceneObject::Detach(ObjectComponent* Component)
{
	for (auto i = ChildComponents.begin(); i < ChildComponents.end(); i++)
	{
		if (*i == Component)
		{
			ChildComponents.erase(i);
			Component->DetachThis();
			delete Component;
			return;
		}
	}
	ENGINE_UNREACHABLE();
}

void engine::SceneObject::ClearComponents()
{
	while (ChildComponents.size())
	{
		Detach(ChildComponents[0]);
	}
	ChildComponents.clear();
}

void engine::SceneObject::CheckTransform()
{
	if (Position != OldPosition
		|| Rotation != OldRotation
		|| Scale != OldScale)
	{
		OldPosition = Position;
		OldRotation = Rotation;
		OldScale = Scale;

		ObjectTransform = Transform(Position, Rotation, Scale);
	}
}

void engine::SceneObject::CheckComponentTransform()
{
	for (auto& i : ChildComponents)
	{
		i->UpdateTransform();
	}
}

void engine::SceneObject::Destroy()
{
	OriginScene->DestroyedObjects.insert(this);
}

void engine::SceneObject::InitObj(Scene* Scn, bool CallBegin, int32 TypeID)
{
	OriginScene = Scn;
	this->TypeID = TypeID;
	if (CallBegin)
	{
		this->Name = Reflection::ObjectTypes[TypeID].Name;
		CheckTransform();
		Begin();
		BeginCalled = true;
	}
}

void engine::SceneObject::UpdateObject()
{
	Update();
	CheckTransform();
	
	for (ObjectComponent* c : ChildComponents)
	{
		c->UpdateAll();
	}
}

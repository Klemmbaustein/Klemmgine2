#pragma once
#include <Engine/File/SerializedData.h>
#include <Engine/Transform.h>
#include "Reflection/ObjectReflection.h"
#include "Reflection/ObjectPropery.h"
#include "Components/ObjectComponent.h"

#ifdef ENGINE_PLUGIN
#define PLUGIN_INTERFACE_FN2(x) {x;}
#define PLUGIN_INTERFACE_FN {}
#else
#define PLUGIN_INTERFACE_FN2(...)
#define PLUGIN_INTERFACE_FN
#endif

namespace engine
{
	class Scene;

	class SceneObject
	{
	public:

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		Transform ObjectTransform;
		ObjectTypeID TypeID = 0;
		string Name;

		friend struct ObjPropertyBase;
		friend class Scene;

		Scene* GetScene()
		{
			return OriginScene;
		}

		std::vector<ObjPropertyBase*> Properties;

#ifndef ENGINE_PLUGIN

		SerializedValue Serialize();
		void DeSerialize(SerializedValue* From);
		void Attach(ObjectComponent* Component);
		void Detach(ObjectComponent* Component);
		void ClearComponents();
		void Destroy();
		void CheckTransform();
#endif

	protected:
		virtual ~SceneObject() PLUGIN_INTERFACE_FN;
		SceneObject()
		{
		}
		
		virtual void Begin() PLUGIN_INTERFACE_FN;
		virtual void OnDestroyed() PLUGIN_INTERFACE_FN;
		virtual void Update() PLUGIN_INTERFACE_FN;

	private:

		std::vector<ObjectComponent*> ChildComponents;
		Vector3 OldPosition;
		Rotation3 OldRotation;
		Vector3 OldScale = 1;

		template<typename T>
		static T* New(Scene* Scn, Vector3 Pos, Rotation3 Rot, Vector3 Scl, bool CallBegin)
		{
			T* Object = new T();
			Object->Position = Pos;
			Object->Rotation = Rot;
			Object->Scale = Scl;
			Object->InitObj(Scn, CallBegin, T::ObjectType);
			return Object;
		}

		void InitObj(Scene* Scn, bool CallBegin, int32 TypeID);

		void UpdateObject();

		SceneObject(SceneObject&) = delete;
		Scene* OriginScene = nullptr;
	};
}
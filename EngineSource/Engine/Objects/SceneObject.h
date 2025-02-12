#pragma once
#include <Core/File/SerializedData.h>
#include <Core/Transform.h>
#include "Reflection/ObjectReflection.h"
#include "Reflection/ObjectPropery.h"
#include "Components/ObjectComponent.h"
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

		/**
		* @brief
		* Returns the scene this object belongs to.
		*/
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
		void CheckComponentTransform();
#endif

	protected:
		virtual ~SceneObject() {};
		SceneObject()
		{
		}

		SceneObject(const SceneObject&) = delete;


		virtual void Begin() {}
		virtual void OnDestroyed() {}
		virtual void Update() {}

	private:

		bool BeginCalled = false;
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
#pragma once
#include "Components/ObjectComponent.h"
#include "Reflection/ObjectPropery.h"
#include "Reflection/ObjectReflection.h"
#include <Core/Event.h>
#include <Core/File/SerializedData.h>
#include <Core/Transform.h>

namespace engine
{
	using ObjectID = uint64;

	class Scene;

	class SceneObject
	{
	public:

		ObjectID ID = 0;

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		ObjectTypeID TypeID = 0;
		Transform ObjectTransform;
		string Name;
		Event<> OnDestroyedEvent;

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
		const std::vector<ObjectComponent*>& GetChildComponents() const
		{
			return this->ChildComponents;
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
		virtual ~SceneObject()
		{
		}
		SceneObject()
		{
		}

		SceneObject(const SceneObject&) = delete;

		virtual void Begin() {}
		virtual void OnDestroyed() {}
		virtual void Update() {}

	private:

		std::vector<ObjectComponent*> ChildComponents;
		bool BeginCalled = false;
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
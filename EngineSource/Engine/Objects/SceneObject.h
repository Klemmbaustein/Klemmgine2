#pragma once
#include <Engine/File/SerializedData.h>
#include <Engine/Graphics/Camera.h>
#include <Engine/Transform.h>
#include "Reflection/ObjectReflection.h"
#include "Reflection/ObjectPropery.h"
#include "Components/ObjectComponent.h"

namespace engine
{
	class Scene;

	class SceneObject : public ISerializable
	{
	public:

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale = 1;
		Transform ObjectTransform;
		ObjectTypeID TypeID = 0;
		string Name;

		friend struct ObjPropertyBase;
		friend class Scene;

		Scene* GetScene();

		std::vector<ObjPropertyBase*> Properties;

		void Attach(ObjectComponent* Component);
		void Detach(ObjectComponent* Component);
		void ClearComponents();
		void Destroy();

	protected:
		void Draw(graphics::Camera* From);
		virtual ~SceneObject();
		SceneObject()
		{
		}
		
		virtual void Begin();
		virtual void OnDestroyed();
		virtual void Update();


	private:

		void CheckTransform();

		std::vector<ObjectComponent*> ChildComponents;
		Vector3 OldPosition;
		Rotation3 OldRotation;
		Vector3 OldScale = 1;

		template<typename T>
		static T* New(Scene* Scn, bool CallBegin)
		{
			T* Object = new T();
			Object->InitObj(Scn, CallBegin, T::ObjectType);
			return Object;
		}

		void InitObj(Scene* Scn, bool CallBegin, int32 TypeID);

		void UpdateObject();

		SceneObject(SceneObject&) = delete;
		Scene* OriginScene = nullptr;
	};
}
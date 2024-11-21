#pragma once
#include <Engine/File/SerializedData.h>
#include <Engine/Graphics/Camera.h>
#include "Reflection/ObjectReflection.h"
#include "Reflection/ObjectPropery.h"

namespace engine
{
	class Scene;

	class SceneObject : public ISerializable
	{
	public:

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		Vector3 Position;
		ObjectTypeID TypeID = 0;
		string Name;

		friend struct ObjPropertyBase;
		friend class Scene;

		Scene* GetScene();

		std::vector<ObjPropertyBase*> Properties;

	protected:
		bool HasVisuals = false;

		virtual void Draw(graphics::Camera* From);
		virtual ~SceneObject()
		{
		}
		SceneObject()
		{
		}
		
		virtual void Begin();

	private:

		template<typename T>
		static T* New(Scene* Scn, bool CallBegin)
		{
			T* Object = new T();
			Object->InitObj(Scn, CallBegin, T::ObjectTypeID);
			return Object;
		}

		void InitObj(Scene* Scn, bool CallBegin, int32 TypeID);

		SceneObject(SceneObject& other) = delete;
		Scene* OriginScene = nullptr;
	};
}
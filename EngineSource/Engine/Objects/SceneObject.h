#pragma once
#include <Engine/File/SerializedData.h>

namespace engine
{
	class Scene;

	class SceneObject : public ISerializable
	{
	public:

		using ObjectTypeID = int32;

		virtual SerializedValue Serialize() override;
		virtual void DeSerialize(SerializedValue* From) override;

		Vector3 Position;
		ObjectTypeID TypeID = 0;
		string Name;

		friend class Scene;

		Scene* GetScene();

	protected:
		virtual ~SceneObject()
		{
		}
		SceneObject()
		{
		}
		
		virtual void Begin();

	private:
		template<typename T>
		static T* New(Scene* Scn)
		{
			T* Object = new T();
			Object->OriginScene = Scn;
			Object->Begin();
			return Object;
		}

		SceneObject(SceneObject& other) = delete;
		Scene* OriginScene = nullptr;
	};
}
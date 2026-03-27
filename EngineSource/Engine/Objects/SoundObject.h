#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Objects/Components/SoundComponent.h>

namespace engine
{
	class SoundObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(SoundObject, "Engine");

		SoundObject();
		~SoundObject();

		void Begin() override;
		void OnDestroyed() override;

		ObjProperty<AssetRef> SoundFile = ObjProperty<AssetRef>("Sound file", AssetRef::EmptyAsset("wav"), this);

		SoundComponent* Component = nullptr;
	};
}
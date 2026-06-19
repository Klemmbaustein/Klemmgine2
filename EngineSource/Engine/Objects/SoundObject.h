#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Objects/Components/SoundComponent.h>

namespace engine
{
	class SoundObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(SoundObject, "Engine/SceneObject", "Engine/Sound");

		SoundObject();
		~SoundObject();

		void Begin() override;
		void OnDestroyed() override;

		void LoadFile(AssetRef File);

		ObjProperty<AssetRef> SoundFile = ObjProperty<AssetRef>("Sound file", AssetRef::EmptyAsset("wav"), this);
		ObjProperty<bool> Loop = ObjProperty<bool>("Loop", true, this);
		ObjProperty<bool> Is3D = ObjProperty<bool>("Is 3D", true, this);

		PROPERTY(float, Range, =, 100);
		PROPERTY(float, Volume, =, 1);
		PROPERTY(float, Pitch, =, 1);

		SoundComponent* Component = nullptr;
	};
}
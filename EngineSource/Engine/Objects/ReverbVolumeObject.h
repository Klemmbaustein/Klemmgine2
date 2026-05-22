#pragma once
#include <Engine/Objects/SceneObject.h>
#include <Engine/Sound/Sound.h>

namespace engine
{
	class ReverbVolumeObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(ReverbVolumeObject, "Engine/SceneObject", "Engine/Sound");

		void Begin() override;
		void Update() override;
		void OnDestroyed() override;

	private:
		sound::SoundReverbVolume* Volume = nullptr;
	};
}
#pragma once
#include "ObjectComponent.h"
#include <Engine/File/AssetRef.h>
#include <Engine/Sound/Sound.h>

namespace engine
{
	class SoundComponent : public ObjectComponent
	{
	public:
		SoundComponent();
		~SoundComponent();

		void Load(AssetRef Sound);
		void Play(bool Loop, bool Is3D);
		void Stop();

		void SetRange(float Range);
		void SetVolume(float Volume);
		void SetPitch(float Pitch);

		void Update() override;

		sound::SoundBuffer* Buffer = nullptr;
		sound::SoundSource* Source = nullptr;
		sound::SoundContext* Context = nullptr;

	private:
		void ClearBuffer();
	};
}
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
		void Play();

		void Update() override;

		sound::SoundBuffer* Buffer = nullptr;
		sound::SoundSource* Source = nullptr;
		sound::SoundContext* Context = nullptr;

	private:
		void ClearBuffer();
	};
}
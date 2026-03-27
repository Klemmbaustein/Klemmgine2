#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include "Sources/SoundFileSource.h"
#include <Core/Vector.h>
#include <Engine/Graphics/Camera.h>

namespace engine::sound
{
	struct SoundContext_Private;
	struct SoundBuffer;
	struct SoundSource;

	class SoundContext
	{
	public:

		SoundContext(subsystem::Subsystem* System);

		~SoundContext();

		SoundBuffer* LoadSoundEffect(string Path);
		SoundSource* CreateSoundSource(SoundBuffer* With);

		void SetSourcePosition(SoundSource* Source, Vector3 NewPosition);

		void SetSourceVelocity(SoundSource* Source, Vector3 NewVelocity);

		void Update(graphics::Camera* FromCamera);

		void FreeSoundSource(SoundSource* Source);
		void FreeSoundEffect(SoundBuffer* Buffer);

		void AddFileSource(SoundFileSource* Source);

		subsystem::Subsystem* System = nullptr;
		SoundContext_Private* SoundData = nullptr;

	private:
		std::map<string, SoundBuffer*> Buffers;

		std::vector<SoundFileSource*> Sources;
	};
}
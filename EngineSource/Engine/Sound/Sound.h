#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include "Sources/SoundFileSource.h"
#include <Core/Vector.h>
#include <Engine/Graphics/Camera.h>

namespace engine::sound
{
	struct SoundDevice_Private;
	struct SoundContext_Private;
	struct SoundBuffer;
	struct SoundSource;

	class SoundDevice
	{
	public:
		SoundDevice(subsystem::Subsystem* System);
		~SoundDevice();

		SoundDevice_Private* DeviceData = nullptr;
	};

	class SoundContext
	{
	public:

		SoundContext(SoundDevice* Device);

		~SoundContext();

		SoundBuffer* LoadSoundEffect(string Path);
		SoundSource* CreateSoundSource(SoundBuffer* With);

		void SetSourcePosition(SoundSource* Source, Vector3 NewPosition);

		void SetSourceVelocity(SoundSource* Source, Vector3 NewVelocity);
		void PlaySource(SoundSource* Source, bool Loop);
		void StopSource(SoundSource* Source);

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
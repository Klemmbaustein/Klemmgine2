#pragma once
#include "SoundFileSource.h"

namespace engine::sound
{
	class FlacSoundFileSource : public SoundFileSource
	{
	public:
		FlacSoundFileSource();
		~FlacSoundFileSource();

		// Inherited via SoundFileSource
		SoundData* ParseSoundFile(ReadOnlyBufferStream* Stream) override;

	};
}
#pragma once
#include "SoundFileSource.h"

namespace engine::sound
{
	class WavSoundData : public SoundData
	{
	public:
		std::vector<uByte> Bytes;
	};

	class WavSoundFileSource : public SoundFileSource
	{
	public:
		WavSoundFileSource();
		~WavSoundFileSource();

		// Inherited via SoundFileSource
		SoundData* ParseSoundFile(IBinaryStream* Stream) override;

	private:
		static bool CheckByteString(IBinaryStream* Stream, const char* String);
	};
}
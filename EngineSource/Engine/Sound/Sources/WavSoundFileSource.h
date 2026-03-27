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
		SoundData* ParseSoundFile(ReadOnlyBufferStream* Stream) override;

	private:
		static bool CheckByteString(ReadOnlyBufferStream* Stream, const char* String);
	};
}
#pragma once
#include "SoundFileSource.h"
#include <Core/File/BitStreamReader.h>

namespace engine::sound
{
	class FlacFileLoader
	{
	public:
		FlacFileLoader(BitStreamReader* Stream);

		std::vector<string> Headers;
		string FlacVendor;

		uint16 MinBlockSize = 0;
		uint16 MaxBlockSize = 0;
		uint32 MinFrameSize = 0;
		uint32 MaxFrameSize = 0;

		uint32 SampleRate = 0;
		uint8 NumChannels = 0;
		uint8 BitsPerSample = 0;
		uint64 SampleCount = 0;

		std::vector<uByte> SoundBytes;

		void ReadBody(BitStreamReader* Stream);

		bool DecodeFrame(BitStreamReader* Stream);
		void DecodeResiduals(BitStreamReader* Stream);
	};

	class FlacLoadException
	{
	public:
		FlacLoadException(const char* ErrorCode)
		{
			this->ErrorCode = ErrorCode;
		}

		const char* ErrorCode;
	};

	class FlacSoundFileSource : public SoundFileSource
	{
	public:
		FlacSoundFileSource();
		~FlacSoundFileSource();

		// Inherited via SoundFileSource
		SoundData* ParseSoundFile(IBinaryStream* Stream) override;
	};
}
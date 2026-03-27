#include "WavSoundFileSource.h"
#include <Core/Log.h>
#include <Core/Error/EngineAssert.h>

using namespace engine::sound;

engine::sound::WavSoundFileSource::WavSoundFileSource()
{
}

engine::sound::WavSoundFileSource::~WavSoundFileSource()
{
}

SoundData* engine::sound::WavSoundFileSource::ParseSoundFile(ReadOnlyBufferStream* Stream)
{
	if (!CheckByteString(Stream, "RIFF"))
	{
		return nullptr;
	}

	int32 ChunkSize = Stream->Get<int32>();

	if (!CheckByteString(Stream, "WAVE"))
	{
		return nullptr;
	}


	auto Result = new WavSoundData();
	int32 BytePerSecond = 0;
	int16 BlockAlign = 0;
	std::vector<uByte>& SoundBytes = Result->Bytes;

	auto ReadChunk = [&] {
		char Chunk[5]{};

		if (!Stream->Read((uByte*)Chunk, 4))
		{
			return false;
		}

		int32 ChunkSize = Stream->Get<int32>();

		if (strncmp(Chunk, "fmt ", 4) == 0)
		{
			uint16 Format = Stream->Get<int16>();

			ENGINE_ASSERT(ChunkSize == 16);

			// No compression support
			if (Format != 1)
			{
				throw std::runtime_error("WAV file contains compressed data");
			}

			int16 ChannelCount = Stream->Get<int16>();

			switch (ChannelCount)
			{
			case 1:
				Result->IsStereo = false;
				break;
			case 2:
				Result->IsStereo = true;
				break;
			default:
				throw std::runtime_error("WAV file contains invalid number of channels");
			}

			Result->SampleRate = Stream->Get<uint32>();
			BytePerSecond = Stream->Get<int32>();
			BlockAlign = Stream->Get<int16>();
			int16 SampleRate = Stream->Get<int16>();

			switch (SampleRate)
			{
			case 8:
				Result->BitDepth = SoundBitDepth::Int8;
				break;
			case 16:
				Result->BitDepth = SoundBitDepth::Int16;
				break;
			case 32:
				Result->BitDepth = SoundBitDepth::Int32;
				break;
			default:
				throw std::runtime_error("WAV file contains unsupported bit depth");
			}
		}
		else if (strncmp(Chunk, "data", 4) == 0)
		{
			size_t OldPosition = SoundBytes.size();
			SoundBytes.resize(OldPosition + ChunkSize);
			Stream->Read(&SoundBytes[OldPosition], ChunkSize);
		}
		else
		{
			Stream->Read(nullptr, ChunkSize);
		}
		return true;
	};

	while (!Stream->IsEmpty())
	{
		if (!ReadChunk())
		{
			return nullptr;
		}
	}

	Result->SoundBytes = Result->Bytes.data();
	Result->NumBytes = Result->Bytes.size();
	return Result;
}

bool engine::sound::WavSoundFileSource::CheckByteString(ReadOnlyBufferStream* Stream, const char* String)
{
	char Buffer[4]{};

	if (!Stream->Read((uByte*)Buffer, sizeof(Buffer)))
	{
		return false;
	}

	return strncmp(Buffer, String, 4) == 0;
}

#pragma once
#include <Core/Types.h>
#include <Core/File/BinaryStream.h>

namespace engine::sound
{
	enum class SoundBitDepth
	{
		Int8,
		Int16,
		Int32,
		Float32,
	};

	class SoundData
	{
	public:
		uByte* SoundBytes = nullptr;
		size_t NumBytes = 0;
		uint32 SampleRate = 0;
		SoundBitDepth BitDepth = SoundBitDepth::Int16;
		bool IsStereo = false;

		virtual ~SoundData() = default;
	};

	class SoundFileSource
	{
	public:
		virtual SoundData* ParseSoundFile(IBinaryStream* Stream) = 0;

		virtual ~SoundFileSource() = default;
	};
}
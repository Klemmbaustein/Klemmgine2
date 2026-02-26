#pragma once
#include <Core/Types.h>
#include <Core/File/BinaryStream.h>

namespace engine::source
{
	enum class SoundBitDepth
	{
		Int16,
		Int32,
		Float32,
	};

	class SoundData
	{
	public:
		uByte* SoundBytes = nullptr;
		size_t NumBytes = 0;
		SoundBitDepth BitDepth = SoundBitDepth::Int16;
		bool IsStereo = false;

		virtual ~SoundData() = default;
	};

	class SoundFileSource
	{
	public:
		virtual SoundData* ParseSoundFile(ReadOnlyBufferStream* Stream) = 0;

		virtual ~SoundFileSource() = default;
	};
}
#include "FlacSoundFileSource.h"
#include <cstring>

using namespace engine::sound;

engine::sound::FlacSoundFileSource::FlacSoundFileSource()
{
}

engine::sound::FlacSoundFileSource::~FlacSoundFileSource()
{
}

SoundData* engine::sound::FlacSoundFileSource::ParseSoundFile(ReadOnlyBufferStream* Stream)
{
	char Header[4];
	Stream->Read((uByte*)Header, 4);

	if (strncmp(Header, "FLaC", 4) != 0)
	{
		return nullptr;
	}

	return nullptr;
}

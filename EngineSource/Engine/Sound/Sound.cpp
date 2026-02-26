#include "Sound.h"
#include <AL/al.h>
#include <AL/alc.h>

// TODO: fully implement

using namespace engine::subsystem;

namespace engine::sound
{
	struct SoundContext_Private
	{
		ALCdevice* Device;
		ALCcontext* Context;
	};
}

engine::sound::SoundContext::SoundContext(subsystem::Subsystem* System)
{
	this->System = System;
	this->SoundData = new SoundContext_Private();

	SoundData->Device = alcOpenDevice(nullptr);

	System->Print(str::Format("Opened sound device: %s",
		alcGetString(SoundData->Device, ALC_DEVICE_SPECIFIER)),
		Subsystem::LogType::Note);

	SoundData->Context = alcCreateContext(SoundData->Device, nullptr);
}

engine::sound::SoundContext::~SoundContext()
{
	delete this->SoundData;
}

#include "SoundSubsystem.h"
#include <AL/al.h>
#include <AL/alc.h>

engine::sound::SoundSubsystem::SoundSubsystem()
	: Subsystem("Sound", Log::LogColor::Red)
{
	ALCdevice* device = alcOpenDevice(nullptr);

	Print(str::Format("Opened sound device: %s", alcGetString(device, ALC_DEVICE_SPECIFIER)), LogType::Note);

	alcCreateContext(device, nullptr);
}

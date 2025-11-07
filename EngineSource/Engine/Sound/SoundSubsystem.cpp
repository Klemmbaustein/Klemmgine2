#include "SoundSubsystem.h"
#include <AL/al.h>
#include <AL/alc.h>

engine::sound::SoundSubsystem::SoundSubsystem()
	: Subsystem("Sound", Log::LogColor::Red)
{
}

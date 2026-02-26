#include "SoundSubsystem.h"

engine::sound::SoundSubsystem::SoundSubsystem()
	: Subsystem("Sound", Log::LogColor::Red)
{
	MainContext = new SoundContext(this);
}

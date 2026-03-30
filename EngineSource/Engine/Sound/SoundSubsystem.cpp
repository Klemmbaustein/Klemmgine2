#include "SoundSubsystem.h"

engine::sound::SoundSubsystem::SoundSubsystem()
	: Subsystem("Sound", Log::LogColor::Red)
{
	MainDevice = new SoundDevice(this);

	MainContext = new SoundContext(MainDevice);
}

engine::sound::SoundSubsystem::~SoundSubsystem()
{
	delete MainContext;
	delete MainDevice;
}

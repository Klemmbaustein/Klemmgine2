#pragma once
#include <Engine/Subsystem/Subsystem.h>
#include "Sound.h"

namespace engine::sound
{
	class SoundSubsystem : public subsystem::Subsystem
	{
	public:
		SoundSubsystem();

		~SoundSubsystem();

		SoundDevice* MainDevice = nullptr;
		SoundContext* MainContext = nullptr;
	};
}
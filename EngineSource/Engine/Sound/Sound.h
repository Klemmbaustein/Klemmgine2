#pragma once
#include <Engine/Subsystem/Subsystem.h>

namespace engine::sound
{
	struct SoundContext_Private;

	class SoundContext
	{
	public:

		SoundContext(subsystem::Subsystem* System);

		~SoundContext();

		subsystem::Subsystem* System = nullptr;
		SoundContext_Private* SoundData = nullptr;
	};
}
#pragma once
#include <ds/native/nativeModule.hpp>

namespace engine::script
{
	struct SoundBindings
	{
		ds::ClassType* SoundContext = nullptr;
	};

	SoundBindings AddSoundModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
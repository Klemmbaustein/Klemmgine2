#pragma once
#include <ds/native/nativeModule.hpp>

namespace engine::script
{
	struct GraphicsBindings
	{
		ds::ClassType* EnvironmentType = nullptr;
	};

	GraphicsBindings AddGraphicsModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
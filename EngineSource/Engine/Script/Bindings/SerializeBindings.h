#pragma once
#include <ds/native/nativeModule.hpp>

namespace engine::script
{
	struct SerializeBindings
	{
		ds::ClassType* SerializedValue = nullptr;
	};

	SerializeBindings AddSerializeModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
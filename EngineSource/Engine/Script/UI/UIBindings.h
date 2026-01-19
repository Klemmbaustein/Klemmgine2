#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>

namespace engine::script::ui
{
	struct UIBindings
	{
		ds::ClassType* UITextType = nullptr;
	};

	UIBindings AddUIModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
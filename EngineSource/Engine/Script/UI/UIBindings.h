#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>

namespace engine::script::ui
{
	struct UIBindings
	{
		ds::ClassType* UITextType = nullptr;
		ds::ClassType* UISizeType = nullptr;
	};

	UIBindings AddUIModule(ds::NativeModule& To, ds::NativeModule& BaseModule, ds::LanguageContext* ToContext);
}
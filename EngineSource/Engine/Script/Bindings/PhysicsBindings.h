#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>

namespace engine::script
{
	struct PhysicsBindings
	{
		ds::Type* MotionTypeType = nullptr;
		ds::Type* LayerType = nullptr;
		ds::ClassType* HitResultType = nullptr;
		ds::ClassType* PhysicsManagerType = nullptr;
	};

	PhysicsBindings AddPhysicsModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
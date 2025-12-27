#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>

namespace engine::script
{
	struct MathBindings
	{
		ds::NativeStructType* Vec3 = nullptr;
		ds::NativeStructType* Vec2 = nullptr;
		ds::NativeStructType* Rot = nullptr;
	};

	MathBindings AddMathModule(ds::NativeModule& To);
}
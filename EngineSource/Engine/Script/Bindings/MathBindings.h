#pragma once
#include <ds/native/nativeModule.hpp>
#include <ds/native/nativeStructType.hpp>
#include <Core/Vector.h>
#include <Core/Transform.h>

namespace engine::script
{
	struct MathBindings
	{
		ds::NativeStructType* Vec3 = nullptr;
		ds::NativeStructType* Vec2 = nullptr;
		ds::NativeStructType* Rot = nullptr;
		ds::NativeStructType* Bounds = nullptr;
		ds::NativeStructType* Transform = nullptr;
	};

	struct DecomposeResult
	{
		Vector3 Position;
		Rotation3 Rotation;
		Vector3 Scale;
	};

	MathBindings AddMathModule(ds::NativeModule& To, ds::LanguageContext* ToContext);
}
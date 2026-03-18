#pragma once
#include <Core/Vector.h>

namespace engine::graphics
{
	struct Light
	{
		float Intensity = 0;
		float Range = 1.0f;
		Vector3 Color;
		Vector3 Position;
	};
}
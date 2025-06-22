#pragma once
#include <Core/Vector.h>

namespace engine::graphics
{
	struct Environment
	{
		Vector3 SunColor = Vector3();
		float SunIntensity = 0;
		Vector3 SkyColor;
		Vector3 GroundColor;
	};
}
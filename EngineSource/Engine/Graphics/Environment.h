#pragma once
#include <Core/Vector.h>

namespace engine::graphics
{
	class ShaderObject;

	struct Environment
	{
		Vector3 SunColor = Vector3(1);
		float SunIntensity = 1.0f;
		float AmbientIntensity = 0.2f;
		Vector3 SkyColor = Vector3(0.8f, 0.8f, 1.0f);
		Vector3 GroundColor = Vector3(1.0f, 0.5f, 0.5f);

		void ApplyTo(ShaderObject* TargetShader) const;
	};
}
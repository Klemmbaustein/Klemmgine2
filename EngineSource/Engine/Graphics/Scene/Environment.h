#pragma once
#include <Core/Vector.h>
#include <Core/Transform.h>

namespace engine::graphics
{
	class ShaderObject;

	struct Render
	{
		bool SunShadows = true;
		bool AmbientOcclusion = true;
		bool Bloom  = true;
		bool AntiAlias = true;

		float BloomStrength = 1.0f;
		float BloomThreshold = 0.75f;
		uint32 BloomSamples = 25;
		uint32 BloomShape = 2;
	};

	struct Environment
	{
		Vector3 SunColor = Vector3(1);
		Rotation3 SunRotation = Rotation3(45, 0, 0);
		float SunIntensity = 1.0f;
		float AmbientIntensity = 0.2f;
		Vector3 SkyColor = Vector3(0.8f, 0.8f, 1.0f);
		Vector3 GroundColor = Vector3(0.7f, 0.5f, 0.6f);

		float FogRange = 0.0f;
		float FogStart = 0.0f;
		Vector3 FogColor = Vector3(1);

		Render RenderSettings;

		void ApplyTo(ShaderObject* TargetShader) const;
	};
}
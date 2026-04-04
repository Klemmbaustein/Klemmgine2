#pragma once
#include "ObjectComponent.h"
#include <Engine/Graphics/Light/Light.h>

namespace engine
{
	class LightComponent : public ObjectComponent
	{
	public:

		LightComponent();
		~LightComponent();

		void Update() override;
		void OnAttached() override;

		void SetRange(float NewRange);
		void SetIntensity(float NewIntensity);
		void SetColor(Vector3 NewColor);

	private:
		void UpdateLight(Vector3 At);

		float Range = 5.0f;
		float Intensity = 1.0f;
		Vector3 LastPosition;
		Vector3 Color = 1;
		graphics::Light* LightData = nullptr;
	};
}
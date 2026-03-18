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

	private:
		void UpdateLight(Vector3 At);

		Vector3 LastPosition;
		graphics::Light* LightData = nullptr;
	};
}
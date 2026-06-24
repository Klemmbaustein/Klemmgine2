#pragma once
#include "ObjectComponent.h"
#include <Engine/Graphics/Camera.h>

namespace engine
{
	class CameraComponent : public ObjectComponent
	{
	public:
		CameraComponent();
		~CameraComponent();

		void Update() override;
		void Use();

		void SetFov(float NewFov);
		float GetFov() const;

		Vector3 ScreenToWorld(Vector2 Screen) const;
		Vector3 WorldToScreen(Vector3 World) const;

	protected:
		float Fov = Rotation3::PI / 2;

		graphics::Camera ComponentCamera = graphics::Camera(Fov);
	};
}
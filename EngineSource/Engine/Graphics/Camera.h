#pragma once
#include <glm/glm.hpp>
#include <Core/Vector.h>
#include <Core/Transform.h>

namespace engine::graphics
{
	class Camera
	{
	public:
		float Aspect = 16.0f / 9.0f;
		float FOV = 2.0f;

		Camera(float FOV);
		glm::mat4 View = glm::mat4(1);
		glm::mat4 Projection = glm::mat4(1);

		bool UseTransform = false;
		Vector3 Position;
		Vector3 Rotation;
		Transform CameraTransform;

		Vector3 GetForward() const;

		void Update();
		Vector3 ScreenToWorld(Vector2 Screen) const;
	};
}
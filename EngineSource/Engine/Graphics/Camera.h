#pragma once
#include "Environment.h"
#include <Core/Transform.h>
#include <Core/Vector.h>

namespace engine::graphics
{
	class Camera
	{
	public:
		Camera(float FOV);

		Vector3 GetForward() const;
		Vector3 GetPosition() const;
		void Update();
		Vector3 ScreenToWorld(Vector2 Screen) const;

		float Aspect = 16.0f / 9.0f;
		float FOV = 2.0f;

		glm::mat4 View = glm::mat4(1);
		glm::mat4 Projection = glm::mat4(1);

		Vector3 Position;
		Vector3 Rotation;
		Transform CameraTransform;

		Environment* UsedEnvironment = new Environment();
		bool UseTransform = false;
	};
}
#pragma once
#include <glm/glm.hpp>
#include <Engine/Vector.h>

namespace engine::graphics
{
	class Camera
	{
	public:
		Camera(float FOV);
		glm::mat4 View = glm::mat4(1);
		glm::mat4 Projection = glm::mat4(1);
		Vector3 Position;
		Vector3 Rotation;

		void Update();
	};
}
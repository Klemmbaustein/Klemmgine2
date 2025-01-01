#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

engine::graphics::Camera::Camera(float FOV)
{
	this->FOV = FOV;
}

void engine::graphics::Camera::Update()
{
	Projection = glm::perspective(FOV, Aspect, 0.1f, 1000.0f);
	
	if (UseTransform)
	{
		View = glm::inverse(CameraTransform.Matrix);
	}
	else
	{
		Vector3 Forward = Vector3::Forward(Rotation * Vector3(1, 1, 0));
		Vector3 Up = Vector3::Up(Rotation);
		View = glm::lookAt(glm::vec3(0), glm::vec3(Forward.X, Forward.Y, Forward.Z), glm::vec3(Up.X, Up.Y, Up.Z));
		View = glm::translate(View, -glm::vec3(Position.X, Position.Y, Position.Z));
	}
}

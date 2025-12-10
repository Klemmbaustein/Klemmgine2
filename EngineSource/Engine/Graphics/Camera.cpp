#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

engine::graphics::Camera::Camera(float FOV)
{
	this->FOV = FOV;
}

engine::Vector3 engine::graphics::Camera::GetForward() const
{
	if (UseTransform)
	{
		return CameraTransform.Forward();
	}
	return Vector3::Forward(Rotation * Vector3(1, 1, 0));
}

void engine::graphics::Camera::Update()
{
	Projection = glm::perspective(FOV, Aspect, 0.1f, 5000.0f);

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

engine::Vector3 engine::graphics::Camera::GetPosition() const
{
	if (UseTransform)
	{
		return CameraTransform.ApplyTo(0);
	}
	else
	{
		return this->Position;
	}
}

engine::Vector3 engine::graphics::Camera::ScreenToWorld(Vector2 Screen) const
{
	glm::mat4 InvProjectionView = glm::inverse(Projection * View);
	glm::vec4 Translated = InvProjectionView * glm::vec4(Screen.X, Screen.Y, 1, 1);

	return Vector3(Translated.x, Translated.y, Translated.z);
}

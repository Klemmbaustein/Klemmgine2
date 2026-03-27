#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace engine;

engine::graphics::Camera::Camera(float FOV)
{
	this->FOV = FOV;
}

Vector3 engine::graphics::Camera::GetForward() const
{
	if (UseTransform)
	{
		return CameraTransform.Forward();
	}
	return Vector3::Forward(Rotation * Vector3(1, 1, 0));
}

Vector3 engine::graphics::Camera::GetUp() const
{
	if (UseTransform)
	{
		return CameraTransform.Up();
	}
	return Vector3::Up(Rotation);
}

void engine::graphics::Camera::Update()
{
	Projection = glm::perspective(FOV, Aspect, 0.1f, 5000.0f);

	if (UseTransform)
	{
		View = glm::inverse(CameraTransform.Matrix);

		Collider = FrustumCollider::FromCamera(GetPosition(), CameraTransform.ApplyRotationTo(Vector3(0, 0, -1)),
			CameraTransform.ApplyRotationTo(Vector3(1, 0, 0)), CameraTransform.ApplyRotationTo(Vector3(0, 1, 0)),
			Aspect, FOV, 0.1f, 5000.0f);
	}
	else
	{
		Vector3 Forward = Vector3::Forward(Rotation * Vector3(1, 1, 0));
		Vector3 Up = Vector3::Up(Rotation);
		Vector3 Right = Vector3::Cross(Forward, Up);
		View = glm::lookAt(glm::vec3(0), glm::vec3(Forward.X, Forward.Y, Forward.Z), glm::vec3(Up.X, Up.Y, Up.Z));
		View = glm::translate(View, -glm::vec3(Position.X, Position.Y, Position.Z));
		Collider = FrustumCollider::FromCamera(Position, Forward, Right, Up, Aspect, FOV, 0.1f, 5000.0f);
	}
}

Vector3 engine::graphics::Camera::GetPosition() const
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

Vector3 engine::graphics::Camera::ScreenToWorld(Vector2 Screen) const
{
	glm::mat4 InvProjectionView = glm::inverse(Projection * View);
	glm::vec4 Translated = InvProjectionView * glm::vec4(Screen.X, Screen.Y, 1, 1);

	return Vector3(Translated.x, Translated.y, Translated.z);
}

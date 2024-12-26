#pragma once
#include "Vector.h"
#include <glm/mat4x4.hpp>

namespace engine
{
	struct Rotation3
	{
		static float DegreeToRadian(float Degree);
		static float RadianToDegree(float Degree);

		Rotation3(float P, float Y, float R, bool Radians = false);
		Rotation3(Vector3 Euler, bool Radians = false);
		Rotation3();

		float P = 0, Y = 0, R = 0;

		Vector3 EulerVector(bool Radians = false) const;

		static constexpr float PI = 3.14159265359f;

		bool operator==(const Rotation3& Other) const;
	};

	struct Transform
	{
		Transform(Vector3 Position, Rotation3 Rotation, Vector3 Scale);
		Transform(glm::mat4 Matrix);

		Transform();

		void Translate(Vector3 Offset);
		void Rotate(Rotation3 Rotation);
		void RotateAround(Vector3 Axis, float Amount);
		void Scale(Vector3 NewScale);

		Vector3 ApplyTo(Vector3 Vec);

		Transform Combine(const Transform& Other) const;

		glm::mat4 Matrix;
	};
}
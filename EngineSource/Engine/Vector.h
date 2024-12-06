#pragma once
#include "Types.h"

namespace engine
{
	struct Vector3
	{
		float X = 0;
		float Y = 0;
		float Z = 0;

		Vector3();
		Vector3(float XYZ);
		Vector3(float X, float Y, float Z);

		string ToString() const;

		Vector3 Normalize();
		float Length() const;

		static Vector3 Forward(Vector3 EulerRotation);
		static Vector3 Right(Vector3 EulerRotation);
		static Vector3 Up(Vector3 EulerRotation);

		static Vector3 Cross(Vector3 a, Vector3 b);

		static Vector3 GetScaledAxis(Vector3 Rotation, uint32 Direction);

		Vector3 operator+(const Vector3& Other);
		Vector3& operator+=(const Vector3& Other);
		Vector3 operator-(const Vector3& Other);
		Vector3 operator-();
		Vector3& operator-=(const Vector3& Other);
		Vector3 operator*(const Vector3& Other);
		Vector3 operator/(const Vector3& Other);

		bool operator==(const Vector3& Other) const;

		static Vector3 FromString(string VectorString);

		float& operator[](size_t Index);
		const float& operator[](size_t Index) const;
	};

	struct Vector2
	{
		float X = 0;
		float Y = 0;
		Vector2();
		Vector2(float XY);
		Vector2(float X, float Y);
		Vector2 operator+(const Vector2& Other);
		Vector2 operator*(const Vector2& Other);
	};
}
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

		[[nodiscard]]
		Vector3 Normalize() const;

		[[nodiscard]]
		float Length() const;

		[[nodiscard]]
		static Vector3 Forward(Vector3 EulerRotation);
		[[nodiscard]]
		static Vector3 Right(Vector3 EulerRotation);
		[[nodiscard]]
		static Vector3 Up(Vector3 EulerRotation);

		[[nodiscard]]
		static Vector3 Cross(Vector3 a, Vector3 b);
		[[nodiscard]]
		static float Dot(Vector3 a, Vector3 b);

		[[nodiscard]]
		static Vector3 GetScaledAxis(Vector3 Rotation, uint32 Direction);

		[[nodiscard]]
		static Vector3 SnapToGrid(Vector3 InPosition, float SnapSize);

		Vector3 operator+(const Vector3& Other) const;
		Vector3& operator+=(const Vector3& Other);
		Vector3 operator-(const Vector3& Other) const;
		Vector3 operator-() const;
		Vector3& operator-=(const Vector3& Other);
		Vector3 operator*(const Vector3& Other) const;
		Vector3 operator/(const Vector3& Other) const;
		
		[[nodiscard]]
		static float Distance(const Vector3& a, const Vector3& b);

		bool operator==(const Vector3& Other) const;

		[[nodiscard]]
		static Vector3 FromString(string VectorString);

		[[nodiscard]]
		float& operator[](size_t Index);
		[[nodiscard]]
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
		string ToString() const;
		static Vector2 FromString(string VectorString);
	};
}
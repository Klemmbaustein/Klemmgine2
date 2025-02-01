#include "Vector.h"
#include <cmath>
#include <iostream>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
using namespace engine;

Vector3::Vector3()
{
}

Vector3::Vector3(float XYZ)
{
	this->X = XYZ;
	this->Y = XYZ;
	this->Z = XYZ;
}

engine::Vector3::Vector3(float X, float Y, float Z)
{
	this->X = X;
	this->Y = Y;
	this->Z = Z;
}

string engine::Vector3::ToString() const
{
	return std::to_string(X) + " " + std::to_string(Y) + " " + std::to_string(Z);
}

Vector3 engine::Vector3::Normalize() const
{
	float Len = Length();
	if (Len > 0)
	{
		return *this / Len;
	}
	return Vector3();
}

float engine::Vector3::Length() const
{
	return std::sqrt(X * X + Y * Y + Z * Z);
}

Vector3 engine::Vector3::Forward(Vector3 EulerRotation)
{
	return GetScaledAxis(EulerRotation, 2);
}

Vector3 engine::Vector3::Right(Vector3 EulerRotation)
{
	return GetScaledAxis(EulerRotation, 0);
}

Vector3 engine::Vector3::Up(Vector3 EulerRotation)
{
	return GetScaledAxis(EulerRotation, 1);
}

Vector3 engine::Vector3::Cross(Vector3 a, Vector3 b)
{
	return Vector3(
		a.Y * b.Z - b.Y * a.Z,
		a.Z * b.X - b.Z * a.X,
		a.X * b.Y - b.X * a.Y
	);
}

float engine::Vector3::Dot(Vector3 a, Vector3 b)
{
	float Result = 0;
	for (size_t i = 0; i < 3; i++)
	{
		Result += a[i] * b[i];
	}
	return Result;
}

typedef Vector3 Axes[3];
static void copy(const Axes& from, Axes& to)
{
	for (size_t i = 0; i != 3; ++i)
	{
		for (size_t j = 0; j != 3; ++j)
		{
			to[i][j] = from[i][j];
		}
	}
}
static void mul(Axes& mat, Axes& b)
{
	Axes result = {};
	for (size_t i = 0; i != 3; ++i)
	{
		for (size_t j = 0; j != 3; ++j)
		{
			float sum = 0;
			for (size_t k = 0; k != 3; ++k)
			{
				sum += mat[i][k] * b[k][j];
			}
			result[i][j] = sum;
		}
	}
	copy(result, mat);
}

Vector3 Vector3::GetScaledAxis(Vector3 Rot, uint32 Dir)
{
	float x = -Rot.X;
	float y = Rot.Y;
	float z = -Rot.Z;
	Axes matX = {
		{1,     0,     0 },
		{0, cosf(x),sinf(x)},
		{0,-sinf(x),cosf(x)}
	};
	Axes matY = {
		{cosf(y),0,-sinf(y)},
		{     0,1,      0},
		{sinf(y),0, cosf(y)}
	};
	Axes matZ = {
		{ cosf(z),sinf(z),0},
		{-sinf(z),cosf(z),0},
		{      0,     0,1}
	};
	Axes axes = {
		{1,0,0},
		{0,1,0},
		{0,0,1}
	};
	mul(axes, matX);
	mul(axes, matY);
	mul(axes, matZ);

	return Vector3(axes[Dir][2], axes[Dir][1], axes[Dir][0]).Normalize();
}

Vector3 engine::Vector3::SnapToGrid(Vector3 InPosition, float SnapSize)
{
	return Vector3(
		std::roundf(InPosition.X / SnapSize) * SnapSize,
		std::roundf(InPosition.Y / SnapSize) * SnapSize,
		std::roundf(InPosition.Z / SnapSize) * SnapSize
	);
}

Vector3 engine::Vector3::operator+(const Vector3& Other) const
{
	return Vector3(X + Other.X, Y + Other.Y, Z + Other.Z);
}

Vector3& engine::Vector3::operator+=(const Vector3& Other)
{
	X += Other.X;
	Y += Other.Y;
	Z += Other.Z;
	return *this;
}

Vector3 engine::Vector3::operator-(const Vector3& Other) const
{
	return Vector3(X - Other.X, Y - Other.Y, Z - Other.Z);
}

Vector3 engine::Vector3::operator-() const
{
	return Vector3(-X, -Y, -Z);
}

Vector3& engine::Vector3::operator-=(const Vector3& Other)
{
	X -= Other.X;
	Y -= Other.Y;
	Z -= Other.Z;
	return *this;
}

Vector3 engine::Vector3::operator*(const Vector3& Other) const
{
	return Vector3(X * Other.X, Y * Other.Y, Z * Other.Z);
}

Vector3 engine::Vector3::operator/(const Vector3& Other) const
{
	return Vector3(X / Other.X, Y / Other.Y, Z / Other.Z);
}

float engine::Vector3::Distance(const Vector3& a, const Vector3& b)
{
	return (a - b).Length();
}

bool engine::Vector3::operator==(const Vector3& Other) const
{
	return X == Other.X && Y == Other.Y && Z == Other.Z;
}

Vector3 engine::Vector3::FromString(string VectorString)
{
	std::vector Elements = str::Split(VectorString, " \t\n");

	if (Elements.size() != 3)
		return 0;

	try
	{
		return Vector3(std::stof(Elements.at(0)), std::stof(Elements.at(1)), std::stof(Elements.at(2)));
	}
	catch (std::invalid_argument)
	{
		return 0;
	}
	catch (std::out_of_range)
	{
		return 0;
	}
}

float& engine::Vector3::operator[](size_t Index)
{
	return *(reinterpret_cast<float*>(this) + Index);
}

const float& engine::Vector3::operator[](size_t Index) const
{
	return *(reinterpret_cast<const float*>(this) + Index);
}

engine::Vector2::Vector2()
{
}

engine::Vector2::Vector2(float XY)
{
	this->X = XY;
	this->Y = XY;
}

engine::Vector2::Vector2(float X, float Y)
{
	this->X = X;
	this->Y = Y;
}

Vector2 engine::Vector2::operator+(const Vector2& Other)
{
	return Vector2(X + Other.X, Y + Other.Y);
}

Vector2 engine::Vector2::operator*(const Vector2& Other)
{
	return Vector2(X * Other.X, Y * Other.Y);
}

string engine::Vector2::ToString() const
{
	return std::to_string(X) + " " + std::to_string(Y);
}

Vector2 engine::Vector2::FromString(string VectorString)
{
	std::vector Elements = str::Split(VectorString, " \t\n");

	if (Elements.size() != 2)
		return 0;

	try
	{
		return Vector2(std::stof(Elements.at(0)), std::stof(Elements.at(1)));
	}
	catch (std::invalid_argument)
	{
		return 0;
	}
	catch (std::out_of_range)
	{
		return 0;
	}
}

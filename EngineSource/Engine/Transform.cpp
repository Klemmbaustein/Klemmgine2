#include "Transform.h"
#include <glm/ext/matrix_transform.hpp>

using namespace engine;

engine::Transform::Transform(Vector3 Position, Rotation3 Rotation, Vector3 Scale)
	: Transform()
{
	Matrix = glm::translate(Matrix, glm::vec3(Position.X, Position.Y, Position.Z));
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.P), glm::vec3(1, 0, 0));
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.Y), glm::vec3(0, 1, 0));
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.R), glm::vec3(0, 0, 1));
	Matrix = glm::scale(Matrix, glm::vec3(Scale.X, Scale.Y, Scale.Z));
}

engine::Transform::Transform(glm::mat4 Matrix)
{
	this->Matrix = Matrix;
}

engine::Transform::Transform()
	: Transform(glm::mat4(1))
{
}

void engine::Transform::Translate(Vector3 Offset)
{
	Matrix = glm::translate(Matrix, glm::vec3(Offset.X, Offset.Y, Offset.Z));
}

void engine::Transform::Rotate(Rotation3 Rotation)
{
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.P), glm::vec3(1, 0, 0));
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.Y), glm::vec3(0, 1, 0));
	Matrix = glm::rotate(Matrix, Rotation3::DegreeToRadian(Rotation.R), glm::vec3(0, 0, 1));
}

void engine::Transform::RotateAround(Vector3 Axis, float Amount)
{
	Matrix = glm::rotate(Matrix, Amount, glm::vec3(Axis.X, Axis.Y, Axis.Z));
}

void engine::Transform::Scale(Vector3 NewScale)
{
	Matrix = glm::scale(Matrix, glm::vec3(NewScale.X, NewScale.Y, NewScale.Z));
}

Vector3 engine::Transform::ApplyTo(Vector3 Vec)
{
	glm::vec4 NewVec = Matrix * glm::vec4(Vec.X, Vec.Y, Vec.Z, 1);
	return Vector3(NewVec.x, NewVec.y, NewVec.z);
}

Transform engine::Transform::Combine(const Transform& Other) const
{
	return Transform(Matrix * Other.Matrix);
}

float engine::Rotation3::DegreeToRadian(float Degree)
{
	return Degree / 180.0f * PI;
}

float engine::Rotation3::RadianToDegree(float Degree)
{
	return Degree / PI * 180.0f;
}

engine::Rotation3::Rotation3(float P, float Y, float R, bool Radians)
{
	if (Radians)
	{
		this->P = RadianToDegree(P);
		this->Y = RadianToDegree(Y);
		this->R = RadianToDegree(R);
	}
	else
	{
		this->P = P;
		this->Y = Y;
		this->R = R;
	}
}

engine::Rotation3::Rotation3(Vector3 Euler, bool Radians)
	: Rotation3(Euler.X, Euler.Y, Euler.Z, Radians)
{
}

engine::Rotation3::Rotation3()
{
}

Vector3 engine::Rotation3::EulerVector(bool Radians) const
{
	if (Radians)
		return Vector3(DegreeToRadian(P), DegreeToRadian(Y), DegreeToRadian(R));
	return Vector3(P, Y, R);
}

bool engine::Rotation3::operator==(const Rotation3& Other) const
{
	return P == Other.P && Y == Other.Y && R == Other.R;
}

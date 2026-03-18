#pragma once
#include <glm/matrix.hpp>
#include <array>
#include <Core/Vector.h>
#include <Engine/Graphics/BoundingBox.h>

namespace engine::graphics
{
	struct FrustumPlane
	{
		FrustumPlane(const Vector3& p1, const Vector3& norm)
			: Normal(norm.Normalize()),
			Distance(Vector3::Dot(Normal, p1))
		{
		}
		FrustumPlane()
		{

		}


		Vector3 Normal;
		float Distance = 0;

		float GetSignedDistanceToPoint(const Vector3& Point) const
		{
			return Vector3::Dot(Point, Normal) - Distance;
		}

		void Normalize()
		{
			float Length = sqrt(Normal.X * Normal.X + Normal.Y + Normal.Y + Normal.Z * Normal.Z + Distance * Distance);
			Normal.X /= Length;
			Normal.Y /= Length;
			Normal.Z /= Length;
			Distance /= Length;
		}
	};

	class FrustumCollider
	{
	public:
		FrustumPlane topFace;
		FrustumPlane bottomFace;

		FrustumPlane rightFace;
		FrustumPlane leftFace;

		FrustumPlane farFace;
		FrustumPlane nearFace;

		static FrustumCollider FromCamera(Vector3 Position, Vector3 Front, Vector3 Right, Vector3 Up, float aspect, float fovY,
			float zNear, float zFar)
		{
			FrustumCollider frustum;
			const float halfVSide = zFar * tanf(fovY * .5f);
			const float halfHSide = halfVSide * aspect;
			const Vector3 frontMultiplierFar = Front * zFar;

			frustum.nearFace = { Position + Front * zNear, Front };
			frustum.farFace = { Position + frontMultiplierFar, -Front };
			frustum.rightFace = { Position,
				Vector3::Cross((frontMultiplierFar - Right * halfHSide), Up) };
			frustum.leftFace = { Position,
				Vector3::Cross(Up, frontMultiplierFar + Right * halfHSide) };
			frustum.topFace = { Position,
				Vector3::Cross(Right, frontMultiplierFar - Up * halfVSide) };
			frustum.bottomFace = { Position,
				Vector3::Cross(frontMultiplierFar + Up * halfVSide, Right) };

			return frustum;
		}

		bool OverlapsBounds(const BoundingBox& Bounds) const
		{
			return BoundsIsOnPlane(Bounds, topFace) && BoundsIsOnPlane(Bounds, bottomFace)
				&& BoundsIsOnPlane(Bounds, rightFace) && BoundsIsOnPlane(Bounds, leftFace)
				&& BoundsIsOnPlane(Bounds, farFace) && BoundsIsOnPlane(Bounds, nearFace);
		}

		static bool BoundsIsOnPlane(const BoundingBox& Bounds, const FrustumPlane& Plane)
		{
			// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
			const float r = Bounds.Extents.X * std::abs(Plane.Normal.X)
				+ Bounds.Extents.Y * std::abs(Plane.Normal.Y) + Bounds.Extents.Z * std::abs(Plane.Normal.Z);

			return -r <= Plane.GetSignedDistanceToPoint(Bounds.Position);
		}
	};
}
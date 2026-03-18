#pragma once
#include <Core/Types.h>
#include <Core/Transform.h>
#include <array>

namespace engine::graphics
{
	struct BoundingBox
	{
		[[nodiscard]]
		static inline BoundingBox FromMinMax(Vector3 Min, Vector3 Max)
		{
			Vector3 Diff = (Max - Min) / 2;

			return BoundingBox{
				.Position = Min + Diff,
				.Extents = Diff
			};
		}

		[[nodiscard]]
		bool Overlaps(const BoundingBox& Other) const
		{
			Vector3 Diff = (Position - Other.Position).Abs();

			Vector3 Range = Extents + Other.Extents;

			return Diff.X < Range.X && Diff.Y < Range.Y && Diff.Z < Range.Z;
		}

		[[nodiscard]]
		BoundingBox Translate(const Transform& By)
		{
			std::array<Vector3, 8> Points = {
				Position - Extents,
				Position + Extents,
				Position + Vector3(Extents.X, -Extents.Y, -Extents.Z),
				Position + Vector3(-Extents.X, Extents.Y, -Extents.Z),
				Position + Vector3(-Extents.X, -Extents.Y, Extents.Z),
				Position + Vector3(Extents.X, Extents.Y, -Extents.Z),
				Position + Vector3(Extents.X, -Extents.Y, Extents.Z),
				Position + Vector3(-Extents.X, Extents.Y, Extents.Z),
			};

			Vector3 Min = INFINITY;
			Vector3 Max = -INFINITY;

			for (auto& i : Points)
			{
				i = By.ApplyTo(i);
				Min = Min.Min(i);
				Max = Max.Max(i);
			}

			return FromMinMax(Min, Max);
		}

		Vector3 Position;
		Vector3 Extents;
	};
}
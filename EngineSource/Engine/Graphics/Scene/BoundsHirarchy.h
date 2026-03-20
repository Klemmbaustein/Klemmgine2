#pragma once
#include <list>
#include <Engine/Debug/DebugDraw.h>
#include <Engine/Graphics/BoundingBox.h>
#include <Engine/Graphics/FrustumCollider.h>
#include <span>

namespace engine::graphics
{
	template<typename T>
	class BvhNode
	{
	public:
		BvhNode* A = nullptr;
		BvhNode* B = nullptr;

		BoundingBox Bounds;

		debug::DebugBox* Debug = nullptr;

		std::list<T> Items;

		~BvhNode()
		{
			if (Debug)
			{
				delete Debug;
			}
			if (A)
			{
				delete A;
			}
			if (B)
			{
				delete B;
			}
		}

		BvhNode(std::list<std::pair<T, BoundingBox>> InItems)
		{
			Vector3 Min = INFINITY;
			Vector3 Max = -INFINITY;

			for (auto& [_, bounds] : InItems)
			{
				Min = Min.Min(bounds.Position - bounds.Extents);
				Max = Max.Max(bounds.Position + bounds.Extents);
			}

			this->Bounds = BoundingBox::FromMinMax(Min, Max);

			if (InItems.size() == 1)
			{
				this->Items = { InItems.begin()->first };
				return;
			}

			size_t SplitDirection = 0;

			if (Bounds.Extents[0] > Bounds.Extents[1] && Bounds.Extents[0] > Bounds.Extents[2])
			{
				SplitDirection = 0;
			}
			if (Bounds.Extents[1] >= Bounds.Extents[0] && Bounds.Extents[1] >= Bounds.Extents[2])
			{
				SplitDirection = 1;
			}
			if (Bounds.Extents[2] >= Bounds.Extents[0] && Bounds.Extents[2] >= Bounds.Extents[1])
			{
				SplitDirection = 2;
			}

			float SplitValue = Bounds.Position[SplitDirection];

			std::list<std::pair<T, BoundingBox>> AItems;
			std::list<std::pair<T, BoundingBox>> BItems;

			for (auto& i : InItems)
			{
				if (i.second.Position[SplitDirection] >= SplitValue)
				{
					AItems.push_back(i);
				}
				else
				{
					BItems.push_back(i);
				}
			}
			if (CheckForEmpty(this->Bounds, AItems, BItems))
			{
				return;
			}
			if (CheckForEmpty(this->Bounds, BItems, AItems))
			{
				return;
			}

			if (!AItems.empty())
			{
				this->A = new BvhNode(AItems);
			}

			if (!BItems.empty())
			{
				this->B = new BvhNode(BItems);
			}
		}

		bool CheckForEmpty(const BoundingBox& Bounds, std::list<std::pair<T, BoundingBox>>& AItems,
			std::list<std::pair<T, BoundingBox>>& BItems)
		{
			if (!BItems.empty())
			{
				return false;
			}

			Vector3 Threshold = Bounds.Extents * 0.5f;

			std::list<std::pair<T, BoundingBox>> NewA;

			for (std::pair<T, BoundingBox>& i : AItems)
			{
				if (i.second.Extents.X > Threshold.X
					|| i.second.Extents.Y > Threshold.Y || i.second.Extents.Z > Threshold.Z)
				{
					BItems.push_back(i);
				}
				else
				{
					NewA.push_back(i);
				}
			}

			AItems = NewA;

			if (AItems.empty() != BItems.empty())
			{
				for (auto& i : AItems)
				{
					this->Items.push_back(i.first);
				}
				for (auto& i : BItems)
				{
					this->Items.push_back(i.first);
				}
				return true;
			}
			return false;
		}

		void Query(const BoundingBox& Bounds, std::vector<T>& Result)
		{
			if (this->Bounds.Overlaps(Bounds))
			{
				for (auto& i : Items)
				{
					Result.push_back(i);
				}

				if (A)
				{
					A->Query(Bounds, Result);
				}
				if (B)
				{
					B->Query(Bounds, Result);
				}
			}
		}

		void QueryFrustum(const FrustumCollider& Frustum, std::vector<T>& Result)
		{
			if (Frustum.OverlapsBounds(this->Bounds))
			{
				for (auto& i : Items)
				{
					Result.push_back(i);
				}

				if (A)
				{
					A->QueryFrustum(Frustum, Result);
				}
				if (B)
				{
					B->QueryFrustum(Frustum, Result);
				}
			}
		}
		void QueryBoxes(std::span<BoundingBox> Boxes, std::vector<T>& Result)
		{
			for (auto& i : Boxes)
			{
				if (!i.Overlaps(this->Bounds))
				{
					continue;
				}
				for (auto& itm : Items)
				{
					Result.push_back(itm);
				}

				if (A)
				{
					A->QueryBoxes(Boxes, Result);
				}
				if (B)
				{
					B->QueryBoxes(Boxes, Result);
				}
				return;
			}
		}

	};
}
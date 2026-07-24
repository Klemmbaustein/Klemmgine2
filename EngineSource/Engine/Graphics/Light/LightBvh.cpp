#include "LightBvh.h"
#include <Engine/Scene.h>
#include <Core/ThreadPool.h>
#include <random>
#include <Engine/MainThread.h>

using namespace engine::graphics;

engine::graphics::LightBvh::LightBvh()
{
	AsyncData = std::make_shared<LightBvhAsyncData>();
}

engine::graphics::LightBvh::~LightBvh()
{
	std::lock_guard g{ AsyncData->LightMutex };
	AsyncData->Cancelled = true;
	AsyncData->CurrentRoot = nullptr;
}

void engine::graphics::LightBvh::UpdateBounds(GraphicsScene* With)
{
	if (RequireUpdate && !RunningUpdate)
	{
		for (auto& i : AsyncData->RemovedLight)
		{
			for (auto it = AsyncData->CurrentLights.begin(); it != AsyncData->CurrentLights.end(); it++)
			{
				if (*it == i)
				{
					AsyncData->CurrentLights.erase(it);
					break;
				}
			}
			for (auto it = AsyncData->NewLight.begin(); it != AsyncData->NewLight.end(); it++)
			{
				if (*it == i)
				{
					AsyncData->NewLight.erase(it);
					break;
				}
			}
			delete i;
		}
		AsyncData->RemovedLight.clear();

		for (auto& i : AsyncData->NewLight)
		{
			AsyncData->CurrentLights.push_back(i);
		}

		AsyncData->NewLight.clear();
		RunningUpdate = true;

		ThreadPool::Main()->AddJob([With, this, AsyncData = AsyncData]() {
			std::lock_guard g{ AsyncData->LightMutex };
			if (AsyncData->Cancelled)
			{
				return;
			}

			std::list<std::pair<Light*, BoundingBox>> Bounds;

			for (auto& i : AsyncData->CurrentLights)
			{
				Bounds.push_back({ i, BoundingBox(i->Position, i->Range) });
			}

			auto NewNode = new BvhNode<Light*>(Bounds);

			thread::ExecuteOnMainThread([With, this, NewNode, AsyncData = AsyncData] {
				std::lock_guard g{ AsyncData->LightMutex };
				if (AsyncData->Cancelled)
				{
					return;
				}

				RequireUpdate = ScheduleUpdate;
				ScheduleUpdate = false;
				RunningUpdate = false;

				if (AsyncData->CurrentRoot)
				{
					delete AsyncData->CurrentRoot;
				}

				AsyncData->CurrentRoot = NewNode;
				//ShowDebugNodes(With, CurrentRoot, 0);
			});
		});
	}
}

Light* engine::graphics::LightBvh::AddLight(const Light& NewLight)
{
	RequireUpdate = true;
	auto l = new Light(NewLight);

	if (RunningUpdate)
	{
		ScheduleUpdate = true;
	}

	AsyncData->NewLight.push_back(l);
	return l;
}

void engine::graphics::LightBvh::RemoveLight(Light* ToRemove)
{
	AsyncData->RemovedLight.push_back(ToRemove);

	if (RunningUpdate)
	{
		ScheduleUpdate = true;
	}

	RequireUpdate = true;
}

std::vector<Light*> engine::graphics::LightBvh::GetLights(const BoundingBox& Bounds)
{
	if (!AsyncData->CurrentRoot)
	{
		return {};
	}

	std::vector<Light*> Result;

	AsyncData->CurrentRoot->Query(Bounds, Result);

	return Result;
}

void engine::graphics::LightBvh::ShowDebugNodes(GraphicsScene* With, BvhNode<Light*>* Node, size_t Depth)
{
	if (Node->A)
	{
		ShowDebugNodes(With, Node->A, Depth + 1);
	}
	if (Node->B)
	{
		ShowDebugNodes(With, Node->B, Depth + 1);
	}

	static std::vector<Vector3> Colors = {
		Vector3(1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, 0, 1),
		Vector3(1, 0, 1),
		Vector3(0, 1, 1),
		Vector3(1, 1, 0),
	};

	Node->Debug = new debug::DebugBox(Node->Bounds.Position, Rotation3(0),
		Node->Bounds.Extents - Depth * 0.01f, Colors[Depth % Colors.size()]);

	With->Debug.AddShape(Node->Debug);
}

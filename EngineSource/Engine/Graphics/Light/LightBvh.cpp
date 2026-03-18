#include "LightBvh.h"
#include <Engine/Scene.h>
#include <Core/ThreadPool.h>
#include <random>
#include <Engine/MainThread.h>

using namespace engine::graphics;

engine::graphics::LightBvh::LightBvh()
{
}

engine::graphics::LightBvh::~LightBvh()
{
	delete CurrentRoot;
}

void engine::graphics::LightBvh::UpdateBounds(GraphicsScene* With)
{
	if (RequireUpdate && !RunningUpdate)
	{
		for (auto& i : RemovedLight)
		{
			for (auto it = CurrentLights.begin(); it != CurrentLights.end(); it++)
			{
				if (*it == i)
				{
					CurrentLights.erase(it);
					break;
				}
			}
			for (auto it = NewLight.begin(); it != NewLight.end(); it++)
			{
				if (*it == i)
				{
					NewLight.erase(it);
					break;
				}
			}
		}
		RemovedLight.clear();

		for (auto& i : NewLight)
		{
			CurrentLights.push_back(i);
		}

		NewLight.clear();
		RunningUpdate = true;

		ThreadPool::Main()->AddJob([With, this]() {

			std::list<std::pair<Light*, BoundingBox>> Bounds;

			for (auto& i : CurrentLights)
			{
				Bounds.push_back({i, BoundingBox(i->Position, i->Range)});
			}

			auto NewNode = new BvhNode(Bounds);

			thread::ExecuteOnMainThread([With, this, NewNode] {
				RequireUpdate = ScheduleUpdate;
				ScheduleUpdate = false;
				RunningUpdate = false;

				if (CurrentRoot)
				{
					delete CurrentRoot;
				}

				CurrentRoot = NewNode;
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

	this->NewLight.push_back(l);
	return l;
}

void engine::graphics::LightBvh::RemoveLight(Light* ToRemove)
{
	this->RemovedLight.push_back(ToRemove);

	if (RunningUpdate)
	{
		ScheduleUpdate = true;
	}

	RequireUpdate = true;
}

std::vector<Light*> engine::graphics::LightBvh::GetLights(const BoundingBox& Bounds)
{
	if (!CurrentRoot)
	{
		return {};
	}

	std::vector<Light*> Result;

	CurrentRoot->Query(Bounds, Result);

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

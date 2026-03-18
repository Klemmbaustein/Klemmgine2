#pragma once
#include <list>
#include "Light.h"
#include <Engine/Graphics/BoundingBox.h>
#include <Engine/Graphics/Scene/BoundsHirarchy.h>

namespace engine::graphics
{
	class GraphicsScene;

	class LightBvh
	{
	public:
		LightBvh();
		~LightBvh();

		void UpdateBounds(GraphicsScene* With);

		Light* AddLight(const Light& NewLight);
		void RemoveLight(Light* ToRemove);

		[[nodiscard]]
		std::vector<Light*> GetLights(const BoundingBox& Bounds);

		float MaxRange = 100.0f;

	private:

		std::list<Light*> CurrentLights;
		std::list<Light*> NewLight;
		std::list<Light*> RemovedLight;

		void ShowDebugNodes(GraphicsScene* With, BvhNode<Light*>* Node, size_t Depth = 0);

		BvhNode<Light*>* CurrentRoot = nullptr;

		bool RunningUpdate = false;
		bool RequireUpdate = true;
		bool ScheduleUpdate = false;
	};
}
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

		/**
		 * @brief
		 * Applies lighting data to the given shader.
		 * @param Target
		 * The shader to send the data to.
		 * @param Bounds
		 * The bounding box in which to search for lights.
		 */
		void ApplyToShader(ShaderObject* Target, const BoundingBox& Bounds)
		{
			auto Lights = GetLights(Bounds);

			for (size_t light = 0; light < 8; light++)
			{
				bool Active = light < Lights.size();
				// Actually creating a new string for each light slot for each drawable object ends up being a pretty big performance hit
				// Do this instead to speed things up a bit
				static const char* lights[8] = {
					"u_lights[0].isActive",
					"u_lights[1].isActive",
					"u_lights[2].isActive",
					"u_lights[3].isActive",
					"u_lights[4].isActive",
					"u_lights[5].isActive",
					"u_lights[6].isActive",
					"u_lights[7].isActive",
				};
				static const char* lightsPosition[8] = {
					"u_lights[0].position",
					"u_lights[1].position",
					"u_lights[2].position",
					"u_lights[3].position",
					"u_lights[4].position",
					"u_lights[5].position",
					"u_lights[6].position",
					"u_lights[7].position",
				};
				static const char* lightsRangeIntensity[8] = {
					"u_lights[0].rangeFalloffIntensity",
					"u_lights[1].rangeFalloffIntensity",
					"u_lights[2].rangeFalloffIntensity",
					"u_lights[3].rangeFalloffIntensity",
					"u_lights[4].rangeFalloffIntensity",
					"u_lights[5].rangeFalloffIntensity",
					"u_lights[6].rangeFalloffIntensity",
					"u_lights[7].rangeFalloffIntensity",
				};
				static const char* lightsColor[8] = {
					"u_lights[0].color",
					"u_lights[1].color",
					"u_lights[2].color",
					"u_lights[3].color",
					"u_lights[4].color",
					"u_lights[5].color",
					"u_lights[6].color",
					"u_lights[7].color",
				};

				Target->SetInt(Target->GetUniformLocation(lights[light]), Active);

				if (Active)
				{
					Target->SetVec3(Target->GetUniformLocation(lightsPosition[light]), Lights[light]->Position);
					Target->SetVec3(Target->GetUniformLocation(lightsRangeIntensity[light]),
						Vector3(Lights[light]->Range, 8.0f / Lights[light]->Range, Lights[light]->Intensity));
					Target->SetVec3(Target->GetUniformLocation(lightsColor[light]), Lights[light]->Color);
				}
			}
		}

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
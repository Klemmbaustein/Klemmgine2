#pragma once
#include "SceneObject.h"
#include "Components/LandscapeComponent.h"

namespace engine
{
	class LandscapeObject : public SceneObject
	{
	public:

		void Begin() override;
		void OnDestroyed() override;

		ENGINE_OBJECT(LandscapeObject, "Engine");

	private:
	};
}
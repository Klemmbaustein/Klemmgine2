#pragma once
#include "SceneObject.h"

namespace engine
{
	class LightObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(LightObject, "Engine");

		void Begin() override;
		void OnDestroyed() override;

	private:

	};
}
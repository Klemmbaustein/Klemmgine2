#pragma once
#include "SceneObject.h"

namespace engine
{
	class PlayerObject : public SceneObject
	{
	public:

		ENGINE_OBJECT(PlayerObject, "Game");

		void Begin() override;
	};
}
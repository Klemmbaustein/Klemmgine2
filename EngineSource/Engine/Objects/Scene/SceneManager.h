#pragma once
#include <Engine/Scene.h>

namespace engine
{
	class SceneManager : public SceneObject
	{
	public:
		SceneManager();
		~SceneManager() override;

		void Update();
	};
}
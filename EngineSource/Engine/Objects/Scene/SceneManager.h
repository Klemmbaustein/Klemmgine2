#pragma once
#include <Engine/Objects/SceneObject.h>

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
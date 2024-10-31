#pragma once
#include <vector>
#include "ISubsystem.h"
#include <Engine/Scene.h>

namespace engine::subsystem
{
	class SceneSubsystem : public ISubsystem
	{
	public:
		SceneSubsystem();
		virtual ~SceneSubsystem() override;

		void Update() override;

		std::vector<Scene*> LoadedScenes;

		Scene* Main = nullptr;
	};
}
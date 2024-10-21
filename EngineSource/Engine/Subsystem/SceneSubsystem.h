#pragma once
#include <vector>
#include "Subsystem.h"
#include <Engine/Scene.h>

namespace engine::subsystem
{
	class SceneSubsystem : public Subsystem
	{
	public:
		SceneSubsystem();
		virtual ~SceneSubsystem() override;

		void Update() override;

		std::vector<Scene*> LoadedScenes;

		Scene* Main = nullptr;
	};
}
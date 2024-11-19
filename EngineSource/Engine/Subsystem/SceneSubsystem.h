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

		void LoadSceneAsync(string SceneName);

		void Update() override;

		std::vector<Scene*> LoadedScenes;

		Scene* Main = nullptr;

		static SceneSubsystem* Current;

	private:
		void LoadSceneThread(string SceneName);
	};
}
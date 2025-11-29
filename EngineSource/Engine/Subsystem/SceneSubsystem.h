#pragma once
#include <vector>
#include "Subsystem.h"
#include <Engine/Scene.h>

namespace engine
{
	/**
	* @brief
	* Scene subsystem.
	*
	* Subsystem that manages scenes
	*
	* @see engine::Scene
	*/
	class SceneSubsystem : public subsystem::Subsystem
	{
	public:
		SceneSubsystem();
		virtual ~SceneSubsystem() override;

		void LoadSceneAsync(string SceneName);

		void Update() override;

		std::vector<Scene*> LoadedScenes;

		Scene* Main = nullptr;
		bool IsLoading = false;

		static SceneSubsystem* Current;

	private:
		void LoadSceneThread(string SceneName);
	};
}